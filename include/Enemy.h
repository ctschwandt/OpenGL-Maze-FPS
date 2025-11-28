#ifndef ENEMY_H
#define ENEMY_H

#pragma once

#include "Actor.h"

class Maze;

class Enemy : public Actor
{
public:
    enum class EnemyState
    {
        Idle,
        Chasing
    };

    Enemy();

    void update(const Maze &maze, const glm::vec3 &playerPos, float dt);

    EnemyState state;
};

#endif // ENEMY_H
