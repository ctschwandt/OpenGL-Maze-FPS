#include "Player.h"

#include <algorithm>
#include <cmath>
#include <vector>
#include "Maze.h"
#include "Projectile.h"
#include "Enemy.h"
#include "Globals.h"
#include "mygllib/GLFWInput.h"
#include "mygllib/View.h"

#include <glm/gtx/norm.hpp>
#include <GLFW/glfw3.h>


namespace
{
    const glm::vec3 WORLD_UP(0.0f, 1.0f, 0.0f);
    const float PROJECTILE_SPEED = 20.0f;
    const int PROJECTILE_DAMAGE = 20;

    glm::vec3 forward_from_angles(float yaw, float pitch)
    {
        float cos_pitch = std::cos(pitch);
        float sin_pitch = std::sin(pitch);
        float cos_yaw   = std::cos(yaw);
        float sin_yaw   = std::sin(yaw);

        return glm::normalize(glm::vec3(
            cos_yaw * cos_pitch,
            sin_pitch,
            sin_yaw * cos_pitch
        ));
    }

    glm::vec3 horizontalize(const glm::vec3 & v)
    {
        glm::vec3 h(v.x, 0.0f, v.z);
        float len2 = glm::length2(h);
        return (len2 > 0.0f)
             ? h / std::sqrt(len2)
             : glm::vec3(0.0f);
    }

    float horizontal_speed(const glm::vec3 & velocity)
    {
        return std::sqrt(velocity.x * velocity.x +
                         velocity.z * velocity.z);
    }

    void apply_friction(game::PlayerMovement & state,
                        float friction,
                        float dt)
    {
        glm::vec3 horizontal_vel(state.velocity.x, 0.0f, state.velocity.z);
        float speed = glm::length(horizontal_vel);
        if (speed <= 0.0f)
            return;

        float drop      = speed * friction * dt;
        float new_speed = std::max(speed - drop, 0.0f);
        float scale     = (speed > 0.0f) ? (new_speed / speed) : 0.0f;

        state.velocity.x *= scale;
        state.velocity.z *= scale;
    }

    void accelerate(game::PlayerMovement & state,
                    const glm::vec3 & wish_dir,
                    float wish_speed,
                    float accel,
                    float dt)
    {
        if (wish_speed <= 0.0f)
            return;

        float current_speed = glm::dot(state.velocity, wish_dir);
        float add_speed     = wish_speed - current_speed;
        if (add_speed <= 0.0f)
            return;

        float accel_speed = accel * dt * wish_speed;
        accel_speed       = std::min(accel_speed, add_speed);
        state.velocity  += wish_dir * accel_speed;
    }

    void try_fire(game::PlayerMovement & state,
                  const mygllib::GLFWInput & input,
                  const glm::vec3 & aim_direction,
                  const glm::vec3 & eye_position,
                  float dt)
    {
        state.fireCooldown = std::max(0.0f, state.fireCooldown - dt);

        int button_state = glfwGetMouseButton(input.window(), GLFW_MOUSE_BUTTON_LEFT);
        bool firing      = (button_state == GLFW_PRESS) ||
                           input.key_down(GLFW_KEY_ENTER) ||
                           input.key_down(GLFW_KEY_KP_ENTER);

        if (!firing || state.fireCooldown > 0.0f)
            return;

        glm::vec3 dir = glm::normalize(aim_direction);
        if (glm::length2(dir) == 0.0f)
            dir = glm::vec3(0.0f, 0.0f, -1.0f);

        game::Projectile shot;
        shot.position = eye_position + dir * (game::PLAYER_EYE_RADIUS * 0.5f);
        shot.velocity = dir * PROJECTILE_SPEED;
        shot.damage   = PROJECTILE_DAMAGE;
        shot.fromPlayer = true;

        game::active_projectiles().push_back(shot);
        state.fireCooldown = state.fireRate;
    }
}

namespace game
{
    PlayerMovement & player_movement_state()
    {
        static PlayerMovement state;
        return state;
    }

    void update_player_movement(const mygllib::GLFWInput & input,
                                float dt,
                                mygllib::View & view,
                                const Maze & maze,
                                float tile_scale)
    {
        PlayerMovement & state = player_movement_state();

        // One-time init
        if (!state.initialized)
        {
            glm::vec3 eye_pos(view.eyex(), view.eyey(), view.eyez());
            glm::vec3 forward = horizontalize(forward_from_angles(
                static_cast<float>(view.yaw()),
                static_cast<float>(view.pitch())));
            if (glm::length2(forward) == 0.0f)
                forward = glm::vec3(0.0f, 0.0f, -1.0f);

            state.health       = std::clamp(state.health, 0, state.maxHealth);
            if (state.health == 0)
                state.health = state.maxHealth;
            state.score        = std::max(0, state.score);
            state.damageBuffer = 0.0f;

            glm::vec3 eye_offset = forward * PLAYER_EYE_RADIUS;

            state.position     = glm::vec3(eye_pos.x - eye_offset.x,
                                            eye_pos.y - PLAYER_EYE_HEIGHT,
                                            eye_pos.z - eye_offset.z);
            state.groundHeight = state.position.y;
            //state.collisionRadius = collisionRadius;
            state.initialized  = true;
        }

        // Movement basis: camera-relative in FPS view, world-relative in top-down
        glm::vec3 forward(0.0f, 0.0f, -1.0f);
        glm::vec3 right  (1.0f, 0.0f,  0.0f);
        glm::vec3 aim_forward = forward_from_angles(
            static_cast<float>(view.yaw()),
            static_cast<float>(view.pitch()));

        if (!globals::top_down_view)
        {
            const float yaw   = static_cast<float>(view.yaw());
            const float pitch = static_cast<float>(view.pitch());

            forward = forward_from_angles(yaw, pitch);
            right   = glm::normalize(glm::cross(forward, WORLD_UP));

            // Use horizontal movement only for movement basis
            forward = horizontalize(forward);
            right   = horizontalize(right);

            if (glm::length2(forward) == 0.0f)
                forward = glm::vec3(0.0f, 0.0f, -1.0f);

            if (glm::length2(aim_forward) > 0.0f)
                state.facingDirection = glm::normalize(aim_forward);
        }

        // Build wish direction from WASD
        glm::vec3 wish_dir(0.0f);
        if (input.key_down(GLFW_KEY_W)) wish_dir += forward;
        if (input.key_down(GLFW_KEY_S)) wish_dir -= forward;
        if (input.key_down(GLFW_KEY_D)) wish_dir += right;
        if (input.key_down(GLFW_KEY_A)) wish_dir -= right;

        if (glm::length2(wish_dir) > 0.0f)
            wish_dir = glm::normalize(wish_dir);

        if (globals::top_down_view)
        {
            glm::vec3 arrow_dir(0.0f);

            if (input.key_down(GLFW_KEY_UP))
                arrow_dir.z -= 1.0f;

            if (input.key_down(GLFW_KEY_DOWN))
                arrow_dir.z += 1.0f;

            if (input.key_down(GLFW_KEY_RIGHT))
                arrow_dir.x += 1.0f;

            if (input.key_down(GLFW_KEY_LEFT))
                arrow_dir.x -= 1.0f;

            if (glm::length2(arrow_dir) > 0.0f)
                state.facingDirection = glm::normalize(arrow_dir);
        }

        // Edge-triggered inputs
        bool dash_pressed = input.key_down(GLFW_KEY_LEFT_SHIFT) && !state.dashKeyLast;
        bool jump_pressed = input.key_down(GLFW_KEY_SPACE)      && !state.jumpKeyLast;
        bool crouch_down  = input.key_down(GLFW_KEY_LEFT_CONTROL);

        state.dashKeyLast   = input.key_down(GLFW_KEY_LEFT_SHIFT);
        state.jumpKeyLast   = input.key_down(GLFW_KEY_SPACE);
        state.crouchKeyLast = crouch_down;

        // --- DASH ---
        // Dash: override horizontal velocity and skip friction/accel while active
        if (dash_pressed && !state.dashing)
        {
            glm::vec3 dash_dir = (glm::length2(wish_dir) > 0.0f) ? wish_dir : forward;
            if (glm::length2(dash_dir) == 0.0f)
                dash_dir = glm::vec3(1.0f, 0.0f, 0.0f);

            dash_dir = glm::normalize(glm::vec3(dash_dir.x, 0.0f, dash_dir.z));

            // Preserve vertical velocity, override horizontal
            state.velocity.x = dash_dir.x * state.dashSpeed;
            state.velocity.z = dash_dir.z * state.dashSpeed;

            state.dashing   = true;
            state.dashTimer = state.dashDuration;
            state.sliding   = false;
        }

        if (!state.dashing)
        {
            // --- SLIDE START ---
            if (state.onGround && crouch_down &&
                !state.sliding &&
                horizontal_speed(state.velocity) > state.slideThreshold)
            {
                state.sliding    = true;
                state.slideTimer = state.slideDuration;

                // Optional: small horizontal speed boost to emphasize slide
                glm::vec3 horiz(state.velocity.x, 0.0f, state.velocity.z);
                float speed = glm::length(horiz);
                if (speed > 0.0f)
                {
                    float boost_factor = 1.6f; // tweak or set to 1.0f to disable
                    horiz = (horiz / speed) * (speed * boost_factor);
                    state.velocity.x = horiz.x;
                    state.velocity.z = horiz.z;
                }
            }

            // --- SLIDE UPDATE / STOP ---
            if (state.sliding)
            {
                state.slideTimer -= dt;
                if (state.slideTimer <= 0.0f || !crouch_down || !state.onGround)
                {
                    state.sliding = false;
                }
            }

            // --- FRICTION ---
            float friction = state.frictionAir;
            if (state.onGround)
                friction = state.sliding ? state.slideFriction : state.frictionGround;

            apply_friction(state, friction, dt);

            // --- ACCELERATION ---
            // IMPORTANT: no normal accel while sliding, so slide feels like a glide
            if (!state.sliding && glm::length2(wish_dir) > 0.0f)
            {
                float wish_speed = state.onGround ? state.maxGroundSpeed : state.maxAirSpeed;
                float accel     = state.onGround ? state.accelGround    : state.accelAir;
                accelerate(state, wish_dir, wish_speed, accel, dt);
            }
        }

        // Dash timer
        if (state.dashing)
        {
            state.dashTimer -= dt;
            if (state.dashTimer <= 0.0f)
                state.dashing = false;
        }

        // --- JUMP ---
        if (state.onGround && jump_pressed)
        {
            state.velocity.y = state.jumpSpeed;
            state.onGround   = false;
        }

        // --- GRAVITY ---
        if (!state.onGround)
        {
            state.velocity.y -= state.gravity * dt;
        }

        // --- INTEGRATE POSITION ---
        glm::vec3 new_position = state.position + state.velocity * dt;

        auto collides_with_wall = [&](float world_x, float world_z, float radius) -> bool
        {
            int x0 = static_cast<int>(std::floor((world_x - radius) / tile_scale));
            int x1 = static_cast<int>(std::floor((world_x + radius) / tile_scale));
            int z0 = static_cast<int>(std::floor((world_z - radius) / tile_scale));
            int z1 = static_cast<int>(std::floor((world_z + radius) / tile_scale));

            for (int tr = z0; tr <= z1; ++tr)
            {
                for (int tc = x0; tc <= x1; ++tc)
                {
                    if (maze.is_wall_tile(tr, tc))
                        return true;
                }
            }

            return false;
        };

        // Resolve horizontal collisions axis-by-axis to avoid tunneling into corners
        if (collides_with_wall(new_position.x, state.position.z, state.collisionRadius))
        {
            new_position.x   = state.position.x;
            state.velocity.x = 0.0f;
        }
        if (collides_with_wall(new_position.x, new_position.z, state.collisionRadius))
        {
            new_position.z   = state.position.z;
            state.velocity.z = 0.0f;
        }

        // auto collides_with_enemy = [&](float world_x, float world_z, float radius, const Enemy & enemy) -> bool
        // {
        //     float dx = world_x - enemy.pos.x;
        //     float dz = world_z - enemy.pos.z;
        //     float minDist = radius + enemy.radius;
        //     return (dx * dx + dz * dz) < (minDist * minDist);
        // };

        // no longer check for collision with enemies
        // for (const auto & enemy : active_enemies())
        // {
        //     if (collides_with_enemy(new_position.x, state.position.z, state.collisionRadius, enemy))
        //     {
        //         new_position.x   = state.position.x;
        //         state.velocity.x = 0.0f;
        //     }

        //     if (collides_with_enemy(new_position.x, new_position.z, state.collisionRadius, enemy))
        //     {
        //         new_position.z   = state.position.z;
        //         state.velocity.z = 0.0f;
        //     }
        // }

        // Simple ground collision/response (flat plane at groundHeight)
        if (new_position.y <= state.groundHeight)
        {
            new_position.y = state.groundHeight;
            if (state.velocity.y < 0.0f)
                state.velocity.y = 0.0f;
            state.onGround = true;
        }
        else
        {
            state.onGround = false;
        }

        state.position = new_position;

        float eye_height = PLAYER_EYE_HEIGHT;
        if (state.sliding)
            eye_height *= 0.6f;

        glm::vec3 eye_offset   = forward * PLAYER_EYE_RADIUS;
        glm::vec3 eye_position = glm::vec3(state.position.x + eye_offset.x,
                                          state.position.y + eye_height,
                                          state.position.z + eye_offset.z);

        glm::vec3 aim_direction = globals::top_down_view
                               ? state.facingDirection
                               : aim_forward;

        try_fire(state, input, aim_direction, eye_position, dt);

        // --- SYNC BACK TO VIEW ---
        view.eyex() = eye_position.x;
        view.eyey() = eye_position.y;
        view.eyez() = eye_position.z;
    }

    void Player::update(const Maze & maze, const PlayerInput & input, float dt)
    {
        (void)maze;
        (void)input;

        pos += vel * dt;

        if (fireCooldown > 0.0f)
            fireCooldown = std::max(0.0f, fireCooldown - dt);
    }

}
