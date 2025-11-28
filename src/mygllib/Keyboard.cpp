// File  : Keyboard.cpp
// Author: Cole Schwandt

#include <cmath>
#include <cstdlib>
#include <algorithm>

#include "mygllib/GLFWInput.h"
#include "mygllib/Keyboard.h"
#include "mygllib/SingletonView.h"
#include "mygllib/View.h"

namespace
{
    const float MOVE_SPEED      = 3.0f; // units per second
    const float VERTICAL_SPEED  = 3.0f; // units per second

    // radians per second for keyboard look
    const float TURN_SPEED      = 1.5f; // yaw speed (left/right)
    const float LOOK_SPEED      = 1.5f; // pitch speed (up/down)
}

void mygllib::Keyboard::update_from_input(const GLFWInput & input, float dt)
{
    mygllib::View & view = *(mygllib::SingletonView::getInstance());

    float yaw = view.yaw();
    float fx = std::cos(yaw);
    float fz = std::sin(yaw);
    float rx = -fz;
    float rz =  fx;

    bool moved   = false;
    bool rotated = false;

    if (input.key_down(GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(input.window(), GLFW_TRUE);
    }

    float moveStep     = MOVE_SPEED     * dt;
    float verticalStep = VERTICAL_SPEED * dt;

    // --- Translation (WASD + up/down) ---

    if (input.key_down(GLFW_KEY_W))
    {
        view.eyex() += fx * moveStep;
        view.eyez() += fz * moveStep;
        moved = true;
    }
    if (input.key_down(GLFW_KEY_S))
    {
        view.eyex() -= fx * moveStep;
        view.eyez() -= fz * moveStep;
        moved = true;
    }
    if (input.key_down(GLFW_KEY_A))
    {
        view.eyex() -= rx * moveStep;
        view.eyez() -= rz * moveStep;
        moved = true;
    }
    if (input.key_down(GLFW_KEY_D))
    {
        view.eyex() += rx * moveStep;
        view.eyez() += rz * moveStep;
        moved = true;
    }
    if (input.key_down(GLFW_KEY_SPACE))
    {
        view.eyey() += verticalStep;
        moved = true;
    }
    if (input.key_down(GLFW_KEY_C))
    {
        if (view.eyey() > 1.85)
        {
        view.eyey() -= verticalStep;
        moved = true;
        }
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

    // Only recompute center if actually moved or rotated
    if (moved || rotated)
        view.update_center_from_yaw_pitch();
}
