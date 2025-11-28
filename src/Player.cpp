#include "Player.h"

#include <algorithm>
#include <cmath>

#include "Maze.h"
#include "Physics.h"
#include "InputController.h"

namespace
{
    const float MOVE_SPEED = 2.5f;
    const float JUMP_SPEED = 4.5f;
    const float GRAVITY    = -9.8f;
    const float FLOOR_Y    = 0.0f;
}

Player::Player()
    : Actor(),
      vy(0.0f),
      onGround(true),
      fireCooldown(0.0f)
{
    pos = glm::vec3(1.5f, FLOOR_Y, 1.5f);
    height = 1.8f;
    radius = 0.3f;
}

void Player::update(const Maze &maze, const PlayerInput &input, float dt)
{
    yaw += input.mouseDeltaX * 0.0025f;

    float dx = input.moveRight * MOVE_SPEED * dt;
    float dz = input.moveForward * MOVE_SPEED * dt;

    float sinYaw = std::sin(yaw);
    float cosYaw = std::cos(yaw);

    float worldDx = cosYaw * dx - sinYaw * dz;
    float worldDz = sinYaw * dx + cosYaw * dz;

    Physics::move_horizontal(*this, maze, worldDx, worldDz);

    if (input.jumpPressed && onGround)
    {
        vy = JUMP_SPEED;
        onGround = false;
    }

    Physics::apply_gravity_and_floor(*this, dt, GRAVITY, FLOOR_Y);

    if (fireCooldown > 0.0f)
    {
        fireCooldown = std::max(0.0f, fireCooldown - dt);
    }
}

