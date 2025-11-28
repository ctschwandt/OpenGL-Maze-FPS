#include "Physics.h"

#include <cmath>

#include <algorithm>

#include "Actor.h"
#include "Enemy.h"
#include "Maze.h"
#include "Projectile.h"
#include "Player.h"

void Physics::move_horizontal(Actor &a, const Maze &maze, float dx, float dz)
{
    glm::vec3 newPos = a.pos;
    newPos.x += dx;
    if (!maze.hits_wall(newPos.x, newPos.z, a.radius))
    {
        a.pos.x = newPos.x;
    }

    newPos = a.pos;
    newPos.z += dz;
    if (!maze.hits_wall(newPos.x, newPos.z, a.radius))
    {
        a.pos.z = newPos.z;
    }
}

void Physics::apply_gravity_and_floor(Player &p, float dt, float gravity, float floorY)
{
    p.vy += gravity * dt;
    p.pos.y += p.vy * dt;

    if (p.pos.y < floorY)
    {
        p.pos.y = floorY;
        p.vy = 0.0f;
        p.onGround = true;
    }
    else
    {
        p.onGround = false;
    }
}

bool Physics::actors_overlap(const Actor &a, const Actor &b)
{
    float dx = a.pos.x - b.pos.x;
    float dz = a.pos.z - b.pos.z;
    float distSq = dx * dx + dz * dz;
    float r = a.radius + b.radius;

    bool horizontal = distSq <= r * r;

    float aTop = a.pos.y + a.height;
    float bTop = b.pos.y + b.height;
    bool vertical = (a.pos.y <= bTop) && (b.pos.y <= aTop);

    return horizontal && vertical;
}

bool Physics::projectile_hits_enemy(const Projectile &p, const Enemy &e)
{
    if (!p.alive)
        return false;

    float dx = p.pos.x - e.pos.x;
    float dz = p.pos.z - e.pos.z;
    float distSq = dx * dx + dz * dz;
    float r = p.radius + e.radius;

    return distSq <= r * r;
}

