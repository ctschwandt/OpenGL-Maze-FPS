#include "InputController.h"

#include <GLFW/glfw3.h>

PlayerInput InputController::sample(const mygllib::GLFWInput &input, float dt) const
{
    (void)dt;

    PlayerInput out{};
    out.moveForward = 0.0f;
    out.moveRight   = 0.0f;

    if (input.key_down(GLFW_KEY_W)) out.moveForward += 1.0f;
    if (input.key_down(GLFW_KEY_S)) out.moveForward -= 1.0f;
    if (input.key_down(GLFW_KEY_D)) out.moveRight   += 1.0f;
    if (input.key_down(GLFW_KEY_A)) out.moveRight   -= 1.0f;

    out.jumpPressed  = input.key_down(GLFW_KEY_SPACE);
    out.shootPressed = input.key_down(GLFW_MOUSE_BUTTON_LEFT);
    out.mouseDeltaX  = static_cast<float>(input.mouse_delta_x());
    out.mouseDeltaY  = static_cast<float>(input.mouse_delta_y());

    return out;
}

