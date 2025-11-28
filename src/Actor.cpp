#include "Actor.h"

#include <algorithm>

#include "Maze.h"
#include "PlayerInput.h"

namespace game
{
    void Player::update(const Maze & maze, const PlayerInput & input, float dt)
    {
        (void)maze;
        (void)input;

        pos += vel * dt;

        if (fireCooldown > 0.0f)
            fireCooldown = std::max(0.0f, fireCooldown - dt);
    }
}

