#include "Physics.h"

#include <algorithm>
#include <cmath>

#include "Actor.h"
#include "Enemy.h"
#include "Maze.h"
#include "Player.h"
#include "Projectile.h"

namespace Physics
{
    void move_horizontal(Actor &a, const Maze &maze, float dx, float dz)
    {
        glm::vec3 next = a.pos + glm::vec3(dx, 0.0f, dz);
        if (!maze.hits_wall(next.x, a.pos.z, a.radius))
        {
            a.pos.x = next.x;
        }
        if (!maze.hits_wall(a.pos.x, next.z, a.radius))
        {
            a.pos.z = next.z;
        }
    }

    void apply_gravity_and_floor(Player &p, float dt, float gravity, float floorY)
    {
        p.vy += gravity * dt;
        p.pos.y += p.vy * dt;
        if (p.pos.y <= floorY)
        {
            p.pos.y = floorY;
            p.vy = 0.0f;
            p.onGround = true;
        }
    }

    bool actors_overlap(const Actor &a, const Actor &b)
    {
        float dx = a.pos.x - b.pos.x;
        float dz = a.pos.z - b.pos.z;
        float dist2 = dx * dx + dz * dz;
        float r = a.radius + b.radius;
        bool horizontal = dist2 <= r * r;
        float ay0 = a.pos.y;
        float ay1 = a.pos.y + a.height;
        float by0 = b.pos.y;
        float by1 = b.pos.y + b.height;
        bool vertical = ay1 >= by0 && by1 >= ay0;
        return horizontal && vertical;
    }

    bool projectile_hits_enemy(const Projectile &p, const Enemy &e)
    {
        float dx = p.pos.x - e.pos.x;
        float dz = p.pos.z - e.pos.z;
        float dist2 = dx * dx + dz * dz;
        float r = p.radius + e.radius;
        bool horizontal = dist2 <= r * r;
        float py0 = p.pos.y;
        float py1 = p.pos.y + p.radius * 2.0f;
        float ey0 = e.pos.y;
        float ey1 = e.pos.y + e.height;
        bool vertical = py1 >= ey0 && ey1 >= py0;
        return horizontal && vertical;
    }
}

