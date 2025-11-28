#include "InputController.h"

#include <GLFW/glfw3.h>

PlayerInput InputController::sample(const mygllib::GLFWInput &input, float dt) const
{
    PlayerInput pi{};
    pi.moveForward = 0.0f;
    pi.moveRight = 0.0f;

    if (input.key_down(GLFW_KEY_W))
        pi.moveForward += 1.0f;
    if (input.key_down(GLFW_KEY_S))
        pi.moveForward -= 1.0f;
    if (input.key_down(GLFW_KEY_D))
        pi.moveRight += 1.0f;
    if (input.key_down(GLFW_KEY_A))
        pi.moveRight -= 1.0f;

    pi.jumpPressed = input.key_down(GLFW_KEY_SPACE);
    pi.shootPressed = input.mouse_button_down(GLFW_MOUSE_BUTTON_LEFT);
    pi.mouseDeltaX = input.mouse_delta_x() * dt;
    pi.mouseDeltaY = input.mouse_delta_y() * dt;
    return pi;
}

