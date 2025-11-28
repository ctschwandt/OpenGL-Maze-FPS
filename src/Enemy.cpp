#include "Enemy.h"

#include <cmath>

#include "Maze.h"
#include "Physics.h"

Enemy::Enemy()
    : Actor(),
      state(EnemyState::Idle)
{
    pos = glm::vec3(2.5f, 0.0f, 2.5f);
    radius = 0.35f;
    height = 1.7f;
}

void Enemy::update(const Maze &maze, const glm::vec3 &playerPos, float dt)
{
    (void)maze;
    (void)dt;

    glm::vec3 toPlayer = playerPos - pos;
    yaw = std::atan2(toPlayer.z, toPlayer.x);
}

