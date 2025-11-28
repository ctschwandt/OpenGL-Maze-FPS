#include "PlayerMovement.h"

#include <algorithm>
#include <cmath>

#include <glm/gtx/norm.hpp>

#include "mygllib/GLFWInput.h"
#include "mygllib/View.h"

namespace
{
    constexpr glm::vec3 WORLD_UP(0.0f, 1.0f, 0.0f);

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
}

namespace game
{
    PlayerMovement & player_movement_state()
    {
        static PlayerMovement state;
        return state;
    }

    void update_player_movement(const mygllib::GLFWInput & input, float dt, mygllib::View & view)
    {
        PlayerMovement & state = player_movement_state();

        // One-time init
        if (!state.initialized)
        {
            state.position     = glm::vec3(view.eyex(), view.eyey(), view.eyez());
            state.groundHeight = state.position.y;
            state.initialized  = true;
        }

        // Movement basis from camera yaw/pitch
        const float yaw   = static_cast<float>(view.yaw());
        const float pitch = static_cast<float>(view.pitch());

        glm::vec3 forward = forward_from_angles(yaw, pitch);
        glm::vec3 right   = glm::normalize(glm::cross(forward, WORLD_UP));

        // Use horizontal movement only for movement basis
        forward = horizontalize(forward);
        right   = horizontalize(right);

        // Build wish direction from WASD
        glm::vec3 wishDir(0.0f);
        if (input.key_down(GLFW_KEY_W)) wishDir += forward;
        if (input.key_down(GLFW_KEY_S)) wishDir -= forward;
        if (input.key_down(GLFW_KEY_D)) wishDir += right;
        if (input.key_down(GLFW_KEY_A)) wishDir -= right;

        if (glm::length2(wishDir) > 0.0f)
            wishDir = glm::normalize(wishDir);

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
        state.position += state.velocity * dt;

        // Simple ground collision/response (flat plane at groundHeight)
        if (state.position.y <= state.groundHeight)
        {
            state.position.y = state.groundHeight;
            if (state.velocity.y < 0.0f)
                state.velocity.y = 0.0f;
            state.onGround = true;
        }
        else
        {
            state.onGround = false;
        }

        // --- SYNC BACK TO VIEW ---
        view.eyex() = state.position.x;
        view.eyey() = state.position.y;
        view.eyez() = state.position.z;
    }
}

