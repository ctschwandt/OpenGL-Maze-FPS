#include "Projectile.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include <glm/glm.hpp>

#include "Enemy.h"
#include "Maze.h"
#include "Player.h"

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
            float minY = enemy.pos.y;
            float maxY = enemy.pos.y + enemy.height;

            if (p.position.y < minY || p.position.y > maxY)
                return false;

            glm::vec2 diff(p.position.x - enemy.pos.x, p.position.z - enemy.pos.z);
            float distance2 = glm::dot(diff, diff);
            return distance2 <= enemy.radius * enemy.radius;
        };

        for (auto & p : projectiles)
        {
            p.position      += p.velocity * dt;
            p.remainingLife -= dt;

            if (hits_wall(p.position) || p.remainingLife <= 0.0f)
                p.remainingLife = -1.0f;

            if (p.remainingLife <= 0.0f)
                continue;

            for (auto & enemy : enemies)
            {
                if (!hits_enemy(p, enemy))
                    continue;

                int oldHealth = enemy.health;
                enemy.health -= p.damage;

                if (enemy.health <= 0 && oldHealth > 0 && !playerState.scoreLocked)
                    playerState.score += score_for_enemy(enemy.type());

                p.remainingLife = -1.0f;
                break;
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
