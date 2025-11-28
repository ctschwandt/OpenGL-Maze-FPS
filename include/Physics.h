#ifndef PHYSICS_H
#define PHYSICS_H

#pragma once

#include "myglm.h"

class Actor;
class Player;
class Enemy;
class Maze;
struct Projectile;

namespace Physics
{
    void move_horizontal(Actor &a, const Maze &maze, float dx, float dz);
    void apply_gravity_and_floor(Player &p, float dt, float gravity, float floorY);
    bool actors_overlap(const Actor &a, const Actor &b);
    bool projectile_hits_enemy(const Projectile &p, const Enemy &e);
}

#endif // PHYSICS_H
