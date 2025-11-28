// File  : Keyboard.cpp
// Author: Cole Schwandt

#include <cmath>
#include <cstdlib>
#include <algorithm>

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
