// File  : Keyboard.cpp
// Author: Cole Schwandt

#include <cmath>
#include <cstdlib>
#include <algorithm>

#include <glm/glm.hpp>

#include "mygllib/GLFWInput.h"
#include "mygllib/Keyboard.h"
#include "mygllib/Player.h"
#include "mygllib/SingletonView.h"
#include "mygllib/View.h"

namespace
{
    // radians per second for keyboard look
    const float TURN_SPEED      = 1.5f; // yaw speed (left/right)
    const float LOOK_SPEED      = 1.5f; // pitch speed (up/down)

    const float MAX_SPEED       = 7.0f;
    const float GROUND_ACCEL    = 40.0f;
    const float AIR_ACCEL       = 10.0f;
    const float JUMP_SPEED      = 8.0f;
    const float DASH_SPEED      = 20.0f;
    const float DASH_COOLDOWN   = 0.5f;
    const float GRAVITY         = -20.0f;
    const float GROUND_FRICTION = 8.0f;
    const float SLIDE_FRICTION  = 2.0f;
    const float SLIDE_THRESHOLD = 2.0f;
    const float FLOOR_Y         = 0.0f;

    mygllib::Player player;

    bool playerInitialized = false;
    bool jumpWasDown       = false;
    bool dashWasDown       = false;
    float dashCooldown     = 0.0f;

    void ensure_player_initialized(mygllib::View & view)
    {
        if (playerInitialized)
            return;

        player.pos = glm::vec3(view.eyex(), view.eyey(), view.eyez());
        player.vel = glm::vec3(0.0f);
        player.onGround = true;
        player.sliding = false;
        playerInitialized = true;
    }

    static void apply_ground_or_air_control(const mygllib::GLFWInput & input, float dt)
    {
        mygllib::View & view = *(mygllib::SingletonView::getInstance());
        float yaw = static_cast<float>(view.yaw());
        glm::vec3 forward(std::cos(yaw), 0.0f, std::sin(yaw));
        glm::vec3 right(-forward.z, 0.0f, forward.x);

        glm::vec3 wishDir(0.0f);
        if (input.key_down(GLFW_KEY_W)) wishDir += forward;
        if (input.key_down(GLFW_KEY_S)) wishDir -= forward;
        if (input.key_down(GLFW_KEY_D)) wishDir += right;
        if (input.key_down(GLFW_KEY_A)) wishDir -= right;

        float wishDirLen = glm::length(wishDir);
        if (wishDirLen == 0.0f)
            return;

        wishDir /= wishDirLen;

        float accel = player.onGround ? GROUND_ACCEL : AIR_ACCEL;
        float currentSpeed = glm::dot(player.vel, wishDir);
        float addSpeed = MAX_SPEED - currentSpeed;
        if (addSpeed <= 0.0f)
            return;

        float accelSpeed = accel * dt;
        if (accelSpeed > addSpeed)
            accelSpeed = addSpeed;

        player.vel += wishDir * accelSpeed;
    }

    static void handle_jump(const mygllib::GLFWInput & input, float /*dt*/)
    {
        bool jumpDown = input.key_down(GLFW_KEY_SPACE);
        if (jumpDown && !jumpWasDown && player.onGround)
        {
            player.vel.y = JUMP_SPEED;
            player.onGround = false;
        }

        jumpWasDown = jumpDown;
    }

    static void handle_slide(const mygllib::GLFWInput & input, float /*dt*/)
    {
        bool slideDown = input.key_down(GLFW_KEY_LEFT_CONTROL);
        if (slideDown && player.onGround)
        {
            if (!player.sliding)
            {
                glm::vec3 horizontal(player.vel.x, 0.0f, player.vel.z);
                float speed = glm::length(horizontal);
                if (speed > SLIDE_THRESHOLD)
                {
                    player.sliding = true;
                    player.vel *= 1.1f;
                }
            }
        }
        else
        {
            player.sliding = false;
        }

        if (!player.onGround)
            player.sliding = false;
    }

    static void handle_dash(const mygllib::GLFWInput & input, float dt)
    {
        dashCooldown = std::max(0.0f, dashCooldown - dt);

        bool dashDown = input.key_down(GLFW_KEY_LEFT_SHIFT);
        bool dashPressed = dashDown && !dashWasDown;

        if (dashPressed && dashCooldown <= 0.0f)
        {
            mygllib::View & view = *(mygllib::SingletonView::getInstance());
            float yaw = static_cast<float>(view.yaw());
            glm::vec3 forward(std::cos(yaw), 0.0f, std::sin(yaw));

            player.vel.x = forward.x * DASH_SPEED;
            player.vel.z = forward.z * DASH_SPEED;

            dashCooldown = DASH_COOLDOWN;
        }

        dashWasDown = dashDown;
    }

    static void apply_friction(float dt)
    {
        if (!player.onGround)
            return;

        glm::vec3 horizontal(player.vel.x, 0.0f, player.vel.z);
        float speed = glm::length(horizontal);
        if (speed < 1e-3f)
            return;

        float friction = player.sliding ? SLIDE_FRICTION : GROUND_FRICTION;
        float drop = speed * friction * dt;
        float newSpeed = std::max(0.0f, speed - drop);
        if (speed > 0.0f)
            horizontal *= newSpeed / speed;

        player.vel.x = horizontal.x;
        player.vel.z = horizontal.z;
    }

    void update_player(const mygllib::GLFWInput & input, float dt)
    {
        mygllib::View & view = *(mygllib::SingletonView::getInstance());
        ensure_player_initialized(view);

        apply_ground_or_air_control(input, dt);
        handle_jump(input, dt);
        handle_slide(input, dt);
        handle_dash(input, dt);

        player.vel.y += GRAVITY * dt;

        player.pos += player.vel * dt;

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

void mygllib::Keyboard::update_from_input(const GLFWInput & input, float dt)
{
    mygllib::View & view = *(mygllib::SingletonView::getInstance());

    bool rotated = false;

    if (input.key_down(GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(input.window(), GLFW_TRUE);
    }

    // --- Rotation (arrow keys) ---

    // LEFT / RIGHT -> yaw
    if (input.key_down(GLFW_KEY_RIGHT))
    {
        view.yaw() += TURN_SPEED * dt;   // same sign as mouse moving right
        rotated = true;
    }
    if (input.key_down(GLFW_KEY_LEFT))
    {
        view.yaw() -= TURN_SPEED * dt;
        rotated = true;
    }

    // UP / DOWN -> pitch
    if (input.key_down(GLFW_KEY_UP))
    {
        view.pitch() += LOOK_SPEED * dt; // look up
        rotated = true;
    }
    if (input.key_down(GLFW_KEY_DOWN))
    {
        view.pitch() -= LOOK_SPEED * dt; // look down
        rotated = true;
    }

    // Clamp pitch to same range as mouse
    if (rotated)
    {
        view.pitch() = std::clamp(view.pitch(), -1.2f, 1.2f);
    }

    // Process movement & physics (ground/air accel, dash, slide, gravity)
    update_player(input, dt);

    // Always recompute center when movement/rotation occurred
    view.update_center_from_yaw_pitch();
}

