#ifndef ENEMY_H
#define ENEMY_H

#include <vector>

#include <glm/glm.hpp>

class Maze;

namespace game
{
    struct PlayerMovement;

    enum class EnemyType
    {
        CylinderBot,
        SphereDrone,
        CubeTurret,
        PrismCharger
    };

    struct Bullet
    {
        glm::vec3 pos{0.0f};
        glm::vec3 vel{0.0f};
        float radius{0.2f};
        float ttl{5.0f};
        bool fromPlayer{false};
    };

    enum class PrismChargeState
    {
        Idle,
        Charging
    };

    struct Enemy
    {
        EnemyType type{EnemyType::CylinderBot};
        glm::vec3 pos{0.0f};
        glm::vec3 vel{0.0f};
        float radius{0.8f};
        float height{1.4f};
        int health{60};
        float fireCooldown{0.0f};
        float stateTimer{0.0f};
        float hoverBase{0.0f};
        PrismChargeState chargeState{PrismChargeState::Idle};
    };

    void spawn_default_enemies(std::vector<Enemy> & enemies,
                               const Maze & maze,
                               float tileScale,
                               const glm::vec3 & playerPos);

    void update_enemies(std::vector<Enemy> & enemies,
                        std::vector<Bullet> & bullets,
                        float dt,
                        const Maze & maze,
                        const PlayerMovement & player,
                        float tileScale,
                        float globalTime);

    void update_bullets(std::vector<Bullet> & bullets,
                        std::vector<Enemy> & enemies,
                        float dt,
                        const Maze & maze,
                        PlayerMovement & player,
                        float tileScale);

    void draw_enemies(const std::vector<Enemy> & enemies);
    void draw_bullets(const std::vector<Bullet> & bullets);
}

#endif // ENEMY_H
