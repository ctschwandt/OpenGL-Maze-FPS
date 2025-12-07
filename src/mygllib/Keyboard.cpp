// File  : Keyboard.cpp
// Author: Cole Schwandt

#include <cmath>
#include <cstdlib>
#include <algorithm>

#include "mygllib/GLFWInput.h"
#include "mygllib/Keyboard.h"
#include "mygllib/SingletonView.h"
#include "mygllib/View.h"
#include "Globals.h"

namespace
{
    // radians per second for keyboard look
    const float TURN_SPEED = 1.5f; // yaw speed (left/right)
    const float LOOK_SPEED = 1.5f; // pitch speed (up/down)
}

void mygllib::Keyboard::update_from_input(const GLFWInput & input, float dt)
{
    mygllib::View & view = *(mygllib::SingletonView::getInstance());

    bool rotated = false;

    if (input.key_down(GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(input.window(), GLFW_TRUE);
    }

    if (globals::game_state == globals::GameState::ROBERT_CUBE)
    {
        const float ROT_SPEED_DEG = 90.0f; // degrees per second

        if (input.key_down(GLFW_KEY_RIGHT))
        {
            globals::robert_rot_y += ROT_SPEED_DEG * dt;
        }
        if (input.key_down(GLFW_KEY_LEFT))
        {
            globals::robert_rot_y -= ROT_SPEED_DEG * dt;
        }
        if (input.key_down(GLFW_KEY_UP))
        {
            globals::robert_rot_x += ROT_SPEED_DEG * dt;
        }
        if (input.key_down(GLFW_KEY_DOWN))
        {
            globals::robert_rot_x -= ROT_SPEED_DEG * dt;
        }

        return;
    }

    if (globals::top_down_view)
        return;

    // --- Rotation (arrow keys) ---
    // LEFT / RIGHT -> yaw
    if (input.key_down(GLFW_KEY_RIGHT))
    {
        view.yaw() += TURN_SPEED * dt;
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
        view.pitch() += LOOK_SPEED * dt;
        rotated = true;
    }
    if (input.key_down(GLFW_KEY_DOWN))
    {
        view.pitch() -= LOOK_SPEED * dt;
        rotated = true;
    }

    // clamp pitch to same range as mouse
    if (rotated)
    {
        view.pitch() = std::clamp(view.pitch(), -1.2f, 1.2f);
        view.update_center_from_yaw_pitch();
    }
}
