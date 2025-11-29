#include "Projectile.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include <glm/glm.hpp>

#include "Maze.h"

namespace game
{
    std::vector<Projectile> & active_projectiles()
    {
        static std::vector<Projectile> projectiles;
        return projectiles;
    }

    void update_projectiles(float dt, const Maze & maze, float tileScale)
    {
        auto & projectiles = active_projectiles();

        auto hits_wall = [&](const glm::vec3 & pos) -> bool
        {
            int tc = static_cast<int>(std::floor(pos.x / tileScale));
            int tr = static_cast<int>(std::floor(pos.z / tileScale));
            return maze.is_wall_tile(tr, tc);
        };

        for (auto & p : projectiles)
        {
            p.position      += p.velocity * dt;
            p.remainingLife -= dt;

            if (hits_wall(p.position) || p.remainingLife <= 0.0f)
                p.remainingLife = -1.0f;
        }

        projectiles.erase(
            std::remove_if(projectiles.begin(), projectiles.end(),
                           [](const Projectile & p) { return p.remainingLife <= 0.0f; }),
            projectiles.end());
    }
}
