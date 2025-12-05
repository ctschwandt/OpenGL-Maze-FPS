#include "Player.h"

#include <algorithm>
#include <cmath>
#include <vector>
#include "Maze.h"
#include "Projectile.h"
#include "Globals.h"
#include "mygllib/GLFWInput.h"
#include "mygllib/View.h"

#include <glm/gtx/norm.hpp>
#include <GLFW/glfw3.h>


namespace
{
    constexpr glm::vec3 WORLD_UP(0.0f, 1.0f, 0.0f);
    constexpr float PROJECTILE_SPEED = 80.0f;
    constexpr int PROJECTILE_DAMAGE = 20;

    glm::vec3 forward_from_angles(float yaw, float pitch)
    {
        float cosPitch = std::cos(pitch);
        float sinPitch = std::sin(pitch);
        float cosYaw   = std::cos(yaw);
        float sinYaw   = std::sin(yaw);

        return glm::normalize(glm::vec3(
            cosYaw * cosPitch,
            sinPitch,
            sinYaw * cosPitch
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
        glm::vec3 horizontalVel(state.velocity.x, 0.0f, state.velocity.z);
        float speed = glm::length(horizontalVel);
        if (speed <= 0.0f)
            return;

        float drop     = speed * friction * dt;
        float newSpeed = std::max(speed - drop, 0.0f);
        float scale    = (speed > 0.0f) ? (newSpeed / speed) : 0.0f;

        state.velocity.x *= scale;
        state.velocity.z *= scale;
    }

    void accelerate(game::PlayerMovement & state,
                    const glm::vec3 & wishDir,
                    float wishSpeed,
                    float accel,
                    float dt)
    {
        if (wishSpeed <= 0.0f)
            return;

        float currentSpeed = glm::dot(state.velocity, wishDir);
        float addSpeed     = wishSpeed - currentSpeed;
        if (addSpeed <= 0.0f)
            return;

        float accelSpeed = accel * dt * wishSpeed;
        accelSpeed       = std::min(accelSpeed, addSpeed);
        state.velocity  += wishDir * accelSpeed;
    }

    void try_fire(game::PlayerMovement & state,
                  const mygllib::GLFWInput & input,
                  const glm::vec3 & aimDirection,
                  const glm::vec3 & eyePosition,
                  float dt)
    {
        state.fireCooldown = std::max(0.0f, state.fireCooldown - dt);

        int buttonState = glfwGetMouseButton(input.window(), GLFW_MOUSE_BUTTON_LEFT);
        bool firing     = (buttonState == GLFW_PRESS) ||
                          input.key_down(GLFW_KEY_ENTER) ||
                          input.key_down(GLFW_KEY_KP_ENTER);

        if (!firing || state.fireCooldown > 0.0f)
            return;

        glm::vec3 dir = glm::normalize(aimDirection);
        if (glm::length2(dir) == 0.0f)
            dir = glm::vec3(0.0f, 0.0f, -1.0f);

        game::Projectile shot;
        shot.position = eyePosition + dir * (game::PLAYER_EYE_RADIUS * 0.5f);
        shot.velocity = dir * PROJECTILE_SPEED;
        shot.damage   = PROJECTILE_DAMAGE;

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
                                float tileScale)
    {
        PlayerMovement & state = player_movement_state();

        // One-time init
        if (!state.initialized)
        {
            glm::vec3 eyePos(view.eyex(), view.eyey(), view.eyez());
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

            glm::vec3 eyeOffset = forward * PLAYER_EYE_RADIUS;

            state.position     = glm::vec3(eyePos.x - eyeOffset.x,
                                            eyePos.y - PLAYER_EYE_HEIGHT,
                                            eyePos.z - eyeOffset.z);
            state.groundHeight = state.position.y;
            //state.collisionRadius = collisionRadius;
            state.initialized  = true;
        }

        // Movement basis: camera-relative in FPS view, world-relative in top-down
        glm::vec3 forward(0.0f, 0.0f, -1.0f);
        glm::vec3 right  (1.0f, 0.0f,  0.0f);
        glm::vec3 aimForward = forward_from_angles(
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

            if (glm::length2(aimForward) > 0.0f)
                state.facingDirection = glm::normalize(aimForward);
        }

        // Build wish direction from WASD
        glm::vec3 wishDir(0.0f);
        if (input.key_down(GLFW_KEY_W)) wishDir += forward;
        if (input.key_down(GLFW_KEY_S)) wishDir -= forward;
        if (input.key_down(GLFW_KEY_D)) wishDir += right;
        if (input.key_down(GLFW_KEY_A)) wishDir -= right;

        if (glm::length2(wishDir) > 0.0f)
            wishDir = glm::normalize(wishDir);

        if (globals::top_down_view)
        {
            glm::vec3 arrowDir(0.0f);

            if (input.key_down(GLFW_KEY_UP))
                arrowDir.z -= 1.0f;

            if (input.key_down(GLFW_KEY_DOWN))
                arrowDir.z += 1.0f;

            if (input.key_down(GLFW_KEY_RIGHT))
                arrowDir.x += 1.0f;

            if (input.key_down(GLFW_KEY_LEFT))
                arrowDir.x -= 1.0f;

            if (glm::length2(arrowDir) > 0.0f)
                state.facingDirection = glm::normalize(arrowDir);
        }

        // Edge-triggered inputs
        bool dashPressed = input.key_down(GLFW_KEY_LEFT_SHIFT) && !state.dashKeyLast;
        bool jumpPressed = input.key_down(GLFW_KEY_SPACE)      && !state.jumpKeyLast;
        bool crouchDown  = input.key_down(GLFW_KEY_LEFT_CONTROL);

        state.dashKeyLast   = input.key_down(GLFW_KEY_LEFT_SHIFT);
        state.jumpKeyLast   = input.key_down(GLFW_KEY_SPACE);
        state.crouchKeyLast = crouchDown;

        // --- DASH ---
        // Dash: override horizontal velocity and skip friction/accel while active
        if (dashPressed && !state.dashing)
        {
            glm::vec3 dashDir = (glm::length2(wishDir) > 0.0f) ? wishDir : forward;
            if (glm::length2(dashDir) == 0.0f)
                dashDir = glm::vec3(1.0f, 0.0f, 0.0f);

            dashDir = glm::normalize(glm::vec3(dashDir.x, 0.0f, dashDir.z));

            // Preserve vertical velocity, override horizontal
            state.velocity.x = dashDir.x * state.dashSpeed;
            state.velocity.z = dashDir.z * state.dashSpeed;

            state.dashing   = true;
            state.dashTimer = state.dashDuration;
            state.sliding   = false;
        }

        if (!state.dashing)
        {
            // --- SLIDE START ---
            if (state.onGround && crouchDown &&
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
                    float boostFactor = 1.6f; // tweak or set to 1.0f to disable
                    horiz = (horiz / speed) * (speed * boostFactor);
                    state.velocity.x = horiz.x;
                    state.velocity.z = horiz.z;
                }
            }

            // --- SLIDE UPDATE / STOP ---
            if (state.sliding)
            {
                state.slideTimer -= dt;
                if (state.slideTimer <= 0.0f || !crouchDown || !state.onGround)
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
            if (!state.sliding && glm::length2(wishDir) > 0.0f)
            {
                float wishSpeed = state.onGround ? state.maxGroundSpeed : state.maxAirSpeed;
                float accel     = state.onGround ? state.accelGround    : state.accelAir;
                accelerate(state, wishDir, wishSpeed, accel, dt);
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
        if (state.onGround && jumpPressed)
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
        glm::vec3 newPosition = state.position + state.velocity * dt;

        auto collides_with_wall = [&](float worldX, float worldZ, float radius) -> bool
        {
            int x0 = static_cast<int>(std::floor((worldX - radius) / tileScale));
            int x1 = static_cast<int>(std::floor((worldX + radius) / tileScale));
            int z0 = static_cast<int>(std::floor((worldZ - radius) / tileScale));
            int z1 = static_cast<int>(std::floor((worldZ + radius) / tileScale));

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
        if (collides_with_wall(newPosition.x, state.position.z, state.collisionRadius))
        {
            newPosition.x   = state.position.x;
            state.velocity.x = 0.0f;
        }
        if (collides_with_wall(newPosition.x, newPosition.z, state.collisionRadius))
        {
            newPosition.z   = state.position.z;
            state.velocity.z = 0.0f;
        }

        // Simple ground collision/response (flat plane at groundHeight)
        if (newPosition.y <= state.groundHeight)
        {
            newPosition.y = state.groundHeight;
            if (state.velocity.y < 0.0f)
                state.velocity.y = 0.0f;
            state.onGround = true;
        }
        else
        {
            state.onGround = false;
        }

        state.position = newPosition;

        glm::vec3 eyeOffset   = forward * PLAYER_EYE_RADIUS;
        glm::vec3 eyePosition = glm::vec3(state.position.x + eyeOffset.x,
                                          state.position.y + PLAYER_EYE_HEIGHT,
                                          state.position.z + eyeOffset.z);

        glm::vec3 aimDirection = globals::top_down_view
                               ? state.facingDirection
                               : aimForward;

        try_fire(state, input, aimDirection, eyePosition, dt);

        // --- SYNC BACK TO VIEW ---
        view.eyex() = eyePosition.x;
        view.eyey() = eyePosition.y;
        view.eyez() = eyePosition.z;
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
