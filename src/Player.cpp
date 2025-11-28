#include "Player.h"

#include <cmath>

#include "InputController.h"
#include "Maze.h"
#include "Physics.h"

Player::Player()
    : Actor(),
      vy(0.0f),
      onGround(true),
      fireCooldown(0.0f)
{
}

void Player::update(const Maze &maze, const PlayerInput &input, float dt)
{
    yaw += input.mouseDeltaX;

    float moveSpeed = 2.5f;
    float forward = input.moveForward * moveSpeed * dt;
    float right = input.moveRight * moveSpeed * dt;

    float dx = std::cos(yaw) * forward + std::sin(yaw) * right;
    float dz = -std::sin(yaw) * forward + std::cos(yaw) * right;
    Physics::move_horizontal(*this, maze, dx, dz);

    const float gravity = -9.8f;
    const float floorY = 0.0f;
    if (input.jumpPressed && onGround)
    {
        vy = 5.0f;
        onGround = false;
    }
    Physics::apply_gravity_and_floor(*this, dt, gravity, floorY);

    if (fireCooldown > 0.0f)
    {
        fireCooldown -= dt;
    }
    if (input.shootPressed && fireCooldown <= 0.0f)
    {
        fireCooldown = 0.5f;
    }
}

