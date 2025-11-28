#include "mygllib/Player.h"

#include <algorithm>
#include <cmath>

#include <glm/glm.hpp>

#include "mygllib/GLFWInput.h"
#include "mygllib/SingletonView.h"
#include "mygllib/View.h"

namespace
{
    // Movement tuning constants
    const float MAX_SPEED      = 7.0f;
    const float GROUND_ACCEL   = 40.0f;
    const float AIR_ACCEL      = 10.0f;
    const float JUMP_SPEED     = 8.0f;
    const float DASH_SPEED     = 20.0f;
    const float DASH_COOLDOWN  = 0.5f;
    const float GRAVITY        = -20.0f;
    const float GROUND_FRICTION = 8.0f;
    const float SLIDE_FRICTION  = 2.0f;

    bool playerInitialized = false;
    bool lastJumpDown      = false;
    bool lastDashDown      = false;
    float dashCooldownTimer = 0.0f;

    glm::vec3 horizontal_forward(float yaw)
    {
        glm::vec3 forward(std::cos(yaw), 0.0f, std::sin(yaw));
        float len = std::sqrt(forward.x * forward.x + forward.z * forward.z);
        return (len > 0.0f) ? forward / len : glm::vec3(0.0f);
    }

    void apply_ground_or_air_control(const mygllib::GLFWInput & input, float dt, mygllib::View & view)
    {
        glm::vec3 forward = horizontal_forward(static_cast<float>(view.yaw()));
        glm::vec3 right(-forward.z, 0.0f, forward.x);

        glm::vec3 wishDir(0.0f);
        if (input.key_down(GLFW_KEY_W)) wishDir += forward;
        if (input.key_down(GLFW_KEY_S)) wishDir -= forward;
        if (input.key_down(GLFW_KEY_D)) wishDir += right;
        if (input.key_down(GLFW_KEY_A)) wishDir -= right;

        if (glm::length2(wishDir) == 0.0f)
            return;

        wishDir = glm::normalize(wishDir);
        float accel = mygllib::player.onGround ? GROUND_ACCEL : AIR_ACCEL;

        float currentSpeed = glm::dot(mygllib::player.vel, wishDir);
        float addSpeed = MAX_SPEED - currentSpeed;
        if (addSpeed <= 0.0f)
            return;

        float accelSpeed = accel * dt;
        if (accelSpeed > addSpeed)
            accelSpeed = addSpeed;

        mygllib::player.vel += wishDir * accelSpeed;
    }

    void handle_jump(const mygllib::GLFWInput & input)
    {
        bool jumpDown = input.key_down(GLFW_KEY_SPACE);
        bool jumpPressed = jumpDown && !lastJumpDown;

        if (jumpPressed && mygllib::player.onGround)
        {
            mygllib::player.vel.y = JUMP_SPEED;
            mygllib::player.onGround = false;
        }

        lastJumpDown = jumpDown;
    }

    void handle_slide(const mygllib::GLFWInput & input)
    {
        bool slideDown = input.key_down(GLFW_KEY_LEFT_CONTROL);

        if (slideDown && mygllib::player.onGround)
        {
            glm::vec3 horizontal(mygllib::player.vel.x, 0.0f, mygllib::player.vel.z);
            float speed = glm::length(horizontal);
            if (!mygllib::player.sliding && speed > 2.0f)
            {
                mygllib::player.sliding = true;
                mygllib::player.vel *= 1.1f;
            }
        }
        else if (!slideDown)
        {
            mygllib::player.sliding = false;
        }

        if (!mygllib::player.onGround)
        {
            mygllib::player.sliding = false;
        }
    }

    void handle_dash(const mygllib::GLFWInput & input, float dt, mygllib::View & view)
    {
        bool dashDown = input.key_down(GLFW_KEY_LEFT_SHIFT);
        bool dashPressed = dashDown && !lastDashDown;

        if (dashCooldownTimer > 0.0f)
        {
            dashCooldownTimer -= dt;
            if (dashCooldownTimer < 0.0f)
                dashCooldownTimer = 0.0f;
        }

        if (dashPressed && dashCooldownTimer <= 0.0f)
        {
            glm::vec3 forward = horizontal_forward(static_cast<float>(view.yaw()));
            mygllib::player.vel.x = forward.x * DASH_SPEED;
            mygllib::player.vel.z = forward.z * DASH_SPEED;
            dashCooldownTimer = DASH_COOLDOWN;
        }

        lastDashDown = dashDown;
    }

    void apply_friction(float dt)
    {
        if (!mygllib::player.onGround)
            return;

        glm::vec3 v(mygllib::player.vel.x, 0.0f, mygllib::player.vel.z);
        float speed = glm::length(v);
        if (speed < 1e-3f)
            return;

        float friction = mygllib::player.sliding ? SLIDE_FRICTION : GROUND_FRICTION;
        float drop = speed * friction * dt;
        float newSpeed = std::max(0.0f, speed - drop);
        v *= newSpeed / speed;
        mygllib::player.vel.x = v.x;
        mygllib::player.vel.z = v.z;
    }
}

namespace mygllib
{
    Player player{};

    void update_player(const GLFWInput & input, float dt)
    {
        View & view = *(SingletonView::getInstance());

        if (!playerInitialized)
        {
            player.pos = glm::vec3(view.eyex(), view.eyey(), view.eyez());
            player.vel = glm::vec3(0.0f);
            player.onGround = true;
            player.sliding = false;
            playerInitialized = true;
        }

        apply_ground_or_air_control(input, dt, view);
        handle_jump(input);
        handle_slide(input);
        handle_dash(input, dt, view);

        player.vel.y += GRAVITY * dt;

        player.pos += player.vel * dt;

        const float FLOOR_Y = 0.0f;
        if (player.pos.y < FLOOR_Y)
        {
            player.pos.y = FLOOR_Y;
            if (player.vel.y < 0.0f)
                player.vel.y = 0.0f;
            player.onGround = true;
        }
        else
        {
            player.onGround = false;
        }

        apply_friction(dt);

        view.eyex() = player.pos.x;
        view.eyey() = player.pos.y;
        view.eyez() = player.pos.z;
    }
}

