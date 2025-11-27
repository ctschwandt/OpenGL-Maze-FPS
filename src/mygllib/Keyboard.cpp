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
    const float MOVE_SPEED = 0.2f;
    const float VERTICAL_SPEED = 0.2f;
}

void mygllib::Keyboard::update_from_input(const GLFWInput &input)
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
    if (input.key_down(GLFW_KEY_W))
    {
        view.eyex() += fx * MOVE_SPEED;
        view.eyez() += fz * MOVE_SPEED;
        moved = true;
    }
    if (input.key_down(GLFW_KEY_S))
    {
        view.eyex() -= fx * MOVE_SPEED;
        view.eyez() -= fz * MOVE_SPEED;
        moved = true;
    }
    if (input.key_down(GLFW_KEY_A))
    {
        view.eyex() -= rx * MOVE_SPEED;
        view.eyez() -= rz * MOVE_SPEED;
        moved = true;
    }
    if (input.key_down(GLFW_KEY_D))
    {
        view.eyex() += rx * MOVE_SPEED;
        view.eyez() += rz * MOVE_SPEED;
        moved = true;
    }
    if (input.key_down(GLFW_KEY_SPACE))
    {
        view.eyey() += VERTICAL_SPEED;
        moved = true;
    }
    if (input.key_down(GLFW_KEY_C))
    {
        view.eyey() -= VERTICAL_SPEED;
        moved = true;
    }

    if (moved)
    {
        view.update_center_from_yaw_pitch();
    }
}
