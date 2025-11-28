#include "Enemy.h"

#include <cmath>

#include "Maze.h"
#include "Physics.h"

Enemy::Enemy()
    : Actor(),
      state(EnemyState::Idle)
{
    radius = 0.3f;
    height = 1.6f;
}

void Enemy::update(const Maze &maze, const glm::vec3 &playerPos, float dt)
{
    (void)maze;
    float speed = 1.5f;
    glm::vec3 toPlayer = playerPos - pos;
    if (glm::length(toPlayer) > 0.0001f)
    {
        glm::vec3 dir = glm::normalize(toPlayer);
        pos += glm::vec3(dir.x * speed * dt, 0.0f, dir.z * speed * dt);
        yaw = std::atan2(-dir.z, dir.x);
    }
    state = EnemyState::Chasing;
}

