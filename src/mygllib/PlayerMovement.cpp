#include "mygllib/PlayerMovement.h"

#include <algorithm>
#include <cmath>

#include <GLFW/glfw3.h>
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

        return glm::normalize(glm::vec3(cosYaw * cosPitch, sinPitch, sinYaw * cosPitch));
    }

    glm::vec3 horizontalize(const glm::vec3 & v)
    {
        glm::vec3 h(v.x, 0.0f, v.z);
        float len2 = glm::length2(h);
        return len2 > 0.0f ? h / std::sqrt(len2) : glm::vec3(0.0f);
    }

    float horizontal_speed(const glm::vec3 & velocity)
    {
        return std::sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
    }

    void apply_friction(mygllib::PlayerMovement & state, float friction, float dt)
    {
        glm::vec3 horizontalVel(state.velocity.x, 0.0f, state.velocity.z);
        float speed = glm::length(horizontalVel);
        if (speed <= 0.0f)
            return;

        float drop = speed * friction * dt;
        float newSpeed = std::max(speed - drop, 0.0f);
        float scale = (speed > 0.0f) ? newSpeed / speed : 0.0f;

        state.velocity.x *= scale;
        state.velocity.z *= scale;
    }

    void accelerate(mygllib::PlayerMovement & state, const glm::vec3 & wishDir,
                    float wishSpeed, float accel, float dt)
    {
        float currentSpeed = glm::dot(state.velocity, wishDir);
        float addSpeed = wishSpeed - currentSpeed;
        if (addSpeed <= 0.0f)
            return;

        float accelSpeed = accel * dt * wishSpeed;
        accelSpeed = std::min(accelSpeed, addSpeed);
        state.velocity += wishDir * accelSpeed;
    }
}

namespace mygllib
{
    PlayerMovement & player_movement_state()
    {
        static PlayerMovement state;
        return state;
    }

    void update_player_movement(const GLFWInput & input, float dt, View & view)
    {
        PlayerMovement & state = player_movement_state();

        if (!state.initialized)
        {
            state.position = glm::vec3(view.eyex(), view.eyey(), view.eyez());
            state.groundHeight = state.position.y;
            state.initialized = true;
        }

        // Movement basis from camera yaw/pitch
        const float yaw = view.yaw();
        const float pitch = view.pitch();

        glm::vec3 forward = forward_from_angles(yaw, pitch);
        glm::vec3 right   = glm::normalize(glm::cross(forward, WORLD_UP));

        // Use horizontal movement only
        forward = horizontalize(forward);
        right   = horizontalize(right);

        glm::vec3 wishDir(0.0f);
        if (input.key_down(GLFW_KEY_W)) wishDir += forward;
        if (input.key_down(GLFW_KEY_S)) wishDir -= forward;
        if (input.key_down(GLFW_KEY_D)) wishDir += right;
        if (input.key_down(GLFW_KEY_A)) wishDir -= right;

        if (glm::length2(wishDir) > 0.0f)
            wishDir = glm::normalize(wishDir);

        bool dashPressed   = input.key_down(GLFW_KEY_LEFT_SHIFT) && !state.dashKeyLast;
        bool jumpPressed   = input.key_down(GLFW_KEY_SPACE) && !state.jumpKeyLast;
        bool crouchDown    = input.key_down(GLFW_KEY_LEFT_CONTROL);

        state.dashKeyLast   = input.key_down(GLFW_KEY_LEFT_SHIFT);
        state.jumpKeyLast   = input.key_down(GLFW_KEY_SPACE);
        state.crouchKeyLast = crouchDown;

        // Dash: override horizontal velocity and skip friction/accel while active
        if (dashPressed && !state.dashing)
        {
            glm::vec3 dashDir = (glm::length2(wishDir) > 0.0f) ? wishDir : forward;
            if (glm::length2(dashDir) == 0.0f)
                dashDir = glm::vec3(1.0f, 0.0f, 0.0f);

            dashDir = glm::normalize(glm::vec3(dashDir.x, 0.0f, dashDir.z));
            state.velocity.x = dashDir.x * state.dashSpeed;
            state.velocity.z = dashDir.z * state.dashSpeed;
            state.dashing = true;
            state.dashTimer = state.dashDuration;
            state.sliding = false;
        }

        if (!state.dashing)
        {
            // Slide start/stop
            if (state.onGround && crouchDown &&
                horizontal_speed(state.velocity) > state.slideThreshold)
            {
                state.sliding = true;
                state.slideTimer = state.slideDuration;
            }

            if (state.sliding)
            {
                state.slideTimer -= dt;
                if (state.slideTimer <= 0.0f || !crouchDown)
                    state.sliding = false;
            }

            float friction = state.frictionAir;
            if (state.onGround)
                friction = state.sliding ? state.slideFriction : state.frictionGround;

            apply_friction(state, friction, dt);

            if (glm::length2(wishDir) > 0.0f)
            {
                float wishSpeed = state.onGround ? state.maxGroundSpeed : state.maxAirSpeed;
                float accel = state.onGround ? state.accelGround : state.accelAir;
                accelerate(state, wishDir, wishSpeed, accel, dt);
            }
        }

        if (state.dashing)
        {
            state.dashTimer -= dt;
            if (state.dashTimer <= 0.0f)
                state.dashing = false;
        }

        if (state.onGround && jumpPressed)
        {
            state.velocity.y = state.jumpSpeed;
            state.onGround = false;
        }

        if (!state.onGround)
        {
            state.velocity.y -= state.gravity * dt;
        }

        state.position += state.velocity * dt;

        // Simple ground collision/response
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

        // Sync back to view
        view.eye(state.position.x, state.position.y, state.position.z);
    }
}

