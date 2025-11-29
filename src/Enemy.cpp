#include "Enemy.h"

#include <GL/glew.h>

#include <cmath>

#include <glm/gtx/norm.hpp>

#include "Maze.h"

namespace game
{
    Enemy::Enemy(EnemyType type, float moveSpeed, float detectionRange, int baseHealth)
        : type_(type)
    {
        moveSpeed_ = moveSpeed;
        detectionRange_ = detectionRange;
        health = baseHealth;
    }

    CylinderBot::CylinderBot()
        : Enemy(EnemyType::CylinderBot, 6.0f, 12.0f, 80)
    {
        radius = 0.6f;
        height = 1.6f;
    }

    SphereDrone::SphereDrone()
        : Enemy(EnemyType::SphereDrone, 8.0f, 15.0f, 60)
    {
        radius = 0.7f;
        height = 1.0f;
    }

    CubeTurret::CubeTurret()
        : Enemy(EnemyType::CubeTurret, 0.0f, 20.0f, 120)
    {
        radius = 0.9f;
        height = 1.2f;
    }

    PyramidCharger::PyramidCharger()
        : Enemy(EnemyType::PyramidCharger, 10.0f, 10.0f, 90)
    {
        radius = 0.8f;
        height = 1.4f;
    }

    void draw_cylinder(float radius, float height, int segments = 24);

    void Enemy::update(float dt, const glm::vec3 & playerPos, const Maze & maze)
    {
        switch (type_)
        {
        case EnemyType::CylinderBot:
        {
            constexpr float GROUND_Y   = 0.0f;
            constexpr float TILE_SCALE = 15.0f;

            // CylinderBot stays on the ground plane.
            pos.y = GROUND_Y;

            glm::vec2 dz(playerPos.x - pos.x, playerPos.z - pos.z);
            float dist = glm::length(dz);

            if (dist > detectionRange_)
            {
                vel.x *= 0.9f;
                vel.z *= 0.9f;
                return;
            }

            glm::vec3 dir(playerPos.x - pos.x, 0.0f, playerPos.z - pos.z);
            if (glm::length2(dir) > 0.0001f)
                dir = glm::normalize(dir);

            vel = dir * moveSpeed_;

            auto collides_with_wall = [&](float worldX, float worldZ, float collisionRadius) -> bool
            {
                int x0 = static_cast<int>(std::floor((worldX - collisionRadius) / TILE_SCALE));
                int x1 = static_cast<int>(std::floor((worldX + collisionRadius) / TILE_SCALE));
                int z0 = static_cast<int>(std::floor((worldZ - collisionRadius) / TILE_SCALE));
                int z1 = static_cast<int>(std::floor((worldZ + collisionRadius) / TILE_SCALE));

                for (int tr = z0; tr <= z1; ++tr)
                {
                    for (int tc = x0; tc <= x1; ++tc)
                    {
                        if (maze.is_wall_tile(tr, tc))
                            return true;
                    }
                }

                return false;
            };

            glm::vec3 newPos = pos + vel * dt;

            if (collides_with_wall(newPos.x, pos.z, radius))
            {
                newPos.x = pos.x;
                vel.x    = 0.0f;
            }

            if (collides_with_wall(newPos.x, newPos.z, radius))
            {
                newPos.z = pos.z;
                vel.z    = 0.0f;
            }

            pos = newPos;
            pos.y = GROUND_Y;

            yaw = std::atan2(playerPos.z - pos.z, playerPos.x - pos.x);
            break;
        }

        case EnemyType::SphereDrone:
        case EnemyType::CubeTurret:
        case EnemyType::PyramidCharger:
        default:
            break;
        }
    }

    void Enemy::draw() const
    {
        switch (type_)
        {
        case EnemyType::CylinderBot:
        {
            glPushMatrix();
            glTranslatef(pos.x, pos.y + (height * 0.5f), pos.z);
            glRotatef(yaw * 180.0f / static_cast<float>(M_PI), 0.0f, 1.0f, 0.0f);

            glColor3f(0.2f, 0.6f, 1.0f);
            draw_cylinder(radius, height);

            glPopMatrix();
            break;
        }

        case EnemyType::SphereDrone:
        case EnemyType::CubeTurret:
        case EnemyType::PyramidCharger:
        default:
            break;
        }
    }
}
