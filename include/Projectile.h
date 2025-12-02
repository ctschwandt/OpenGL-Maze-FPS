#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <glm/glm.hpp>
#include <vector>

class Maze;

namespace game
{
    struct Projectile
    {
        glm::vec3 position{0.0f};
        glm::vec3 velocity{0.0f};
        float remainingLife{2.5f};
        int damage{20};
    };

    // Projectiles
    std::vector<Projectile> & active_projectiles();
    void update_projectiles(float dt,
                            const Maze & maze,
                            float tileScale,
                            std::vector<class Enemy> & enemies,
                            class PlayerMovement & playerState);
}

#endif // PROJECTILE_H
