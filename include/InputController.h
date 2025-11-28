#pragma once

#include "myglm.h"
#include "mygllib/GLFWInput.h"

struct PlayerInput
{
    float moveForward;
    float moveRight;
    bool jumpPressed;
    bool shootPressed;
    float mouseDeltaX;
    float mouseDeltaY;
};

class InputController
{
public:
    PlayerInput sample(const mygllib::GLFWInput &input, float dt) const;
};

