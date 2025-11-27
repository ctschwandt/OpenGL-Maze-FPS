// File  : Keyboard.cpp
// Author: Cole Schwandt

#include <cmath>
#include <cstdlib>

#include "mygllib/GLFWInput.h"
#include "mygllib/Keyboard.h"
#include "mygllib/SingletonView.h"
#include "mygllib/View.h"

namespace
{
    const float MOVE_SPEED     = 3.0f; // units per second
    const float VERTICAL_SPEED = 3.0f; // units per second
}

void mygllib::Keyboard::update_from_input(const GLFWInput &input, float dt)
{
    mygllib::View & view = *(mygllib::SingletonView::getInstance());

    float yaw = view.yaw();
    float fx = std::cos(yaw);
    float fz = std::sin(yaw);
    float rx = -fz;
    float rz =  fx;

    bool moved = false;

    if (input.key_down(GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(input.window(), GLFW_TRUE);
    }

    float moveStep     = MOVE_SPEED     * dt;
    float verticalStep = VERTICAL_SPEED * dt;

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
        view.eyey() -= verticalStep;
        moved = true;
    }

    if (moved)
        view.update_center_from_yaw_pitch();
}
