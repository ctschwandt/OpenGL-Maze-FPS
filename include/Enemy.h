#pragma once

#include "Actor.h"

class Maze;

enum class EnemyState
{
    Idle,
    Chasing
};

class Enemy : public Actor
{
public:
    Enemy();

    void update(const Maze &maze, const glm::vec3 &playerPos, float dt);

    EnemyState state;
};

