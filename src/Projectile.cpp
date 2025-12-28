#include "Projectile.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include <glm/glm.hpp>

#include "Enemy.h"
#include "Maze.h"
#include "Player.h"
#include "Globals.h"

namespace game
{
    std::vector<Projectile> & active_projectiles()
    {
        static std::vector<Projectile> projectiles;
        return projectiles;
    }

    void update_projectiles(float dt,
                            const Maze & maze,
                            float tileScale,
                            std::vector<Enemy> & enemies,
                            PlayerMovement & playerState)
    {
        auto & projectiles = active_projectiles();

        auto score_for_enemy = [](EnemyType type) -> int
        {
            switch (type)
            {
                case EnemyType::CylinderBot:   return 1;
                case EnemyType::SphereDrone:   return 2;
                case EnemyType::CubeTurret:    return 2;
                case EnemyType::PyramidCharger:return 4;
            default:                       return 0;
            }
        };

        auto hits_wall = [&](const glm::vec3 & pos) -> bool
        {
            int tc = static_cast<int>(std::floor(pos.x / tileScale));
            int tr = static_cast<int>(std::floor(pos.z / tileScale));
            return maze.is_wall_tile(tr, tc);
        };

        auto hits_enemy = [](const Projectile & p, const Enemy & enemy) -> bool
        {
            if (enemy.type() == EnemyType::SphereDrone)
            {
                glm::vec3 diff = p.position - enemy.pos;
                float distance2 = glm::dot(diff, diff);
                return distance2 <= enemy.radius * enemy.radius;
            }

            float minY = enemy.pos.y;
            float maxY = enemy.pos.y + enemy.height;

            if (p.position.y < minY || p.position.y > maxY)
                return false;

            glm::vec2 diff(p.position.x - enemy.pos.x, p.position.z - enemy.pos.z);
            float distance2 = glm::dot(diff, diff);
            return distance2 <= enemy.radius * enemy.radius;
        };

        auto hits_player = [&](const Projectile & p) -> bool
        {
            float minY = playerState.ground_height;
            float maxY = playerState.ground_height + PLAYER_BODY_HEIGHT;

            if (p.position.y < minY || p.position.y > maxY)
                return false;

            glm::vec2 diff(p.position.x - playerState.position.x,
                           p.position.z - playerState.position.z);

            float distance2 = glm::dot(diff, diff);
            return distance2 <= PLAYER_RADIUS * PLAYER_RADIUS;
        };

        for (auto & p : projectiles)
        {
            p.position      += p.velocity * dt;
            p.remainingLife -= dt;

            if (hits_wall(p.position) || p.remainingLife <= 0.0f)
                p.remainingLife = -1.0f;

            if (p.remainingLife <= 0.0f)
                continue;

            if (p.fromPlayer)
            {
                for (auto & enemy : enemies)
                {
                    if (!hits_enemy(p, enemy))
                        continue;

                    int oldHealth = enemy.health;
                    enemy.health -= p.damage;

                    if (enemy.health <= 0 && oldHealth > 0 &&
                        !globals::enemy_freeze_used_this_run)
                    {
                        playerState.score += score_for_enemy(enemy.type());
                    }

                    p.remainingLife = -1.0f;
                    break;
                }
            }
            else
            {
                if (hits_player(p))
                {
                    playerState.health = std::max(0, playerState.health - p.damage);
                    p.remainingLife    = -1.0f;
                }
            }
        }

        projectiles.erase(
            std::remove_if(projectiles.begin(), projectiles.end(),
                           [](const Projectile & p) { return p.remainingLife <= 0.0f; }),
            projectiles.end());

        enemies.erase(
            std::remove_if(enemies.begin(), enemies.end(),
                           [](const Enemy & e) { return e.health <= 0; }),
            enemies.end());
    }
}
