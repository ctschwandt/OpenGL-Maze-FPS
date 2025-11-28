#ifndef PLAYER_H
#define PLAYER_H

#pragma once

#include "Actor.h"

class Maze;
struct PlayerInput;

class Player : public Actor
{
public:
    Player();

    void update(const Maze &maze, const PlayerInput &input, float dt);

    float vy;
    bool onGround;
    float fireCooldown;
};

#endif // PLAYER_H
