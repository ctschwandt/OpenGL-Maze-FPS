#ifndef ENEMY_H
#define ENEMY_H

#include <array>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Actor.h"
#include "Player.h"

class Maze;

namespace game
{
    enum class EnemyType
    {
        CylinderBot,
        SphereDrone,
        CubeTurret,
        PyramidCharger
    };

    class Enemy : public Actor
    {
    public:
        Enemy(EnemyType type, float pathSpeed, float chaseSpeed, int baseHealth);
        virtual ~Enemy() = default;

        EnemyType type() const { return type_; }
        float path_speed() const { return pathSpeed_; }
        float chase_speed() const { return chaseSpeed_; }

        glm::vec3 previous_position() const { return prevPos; }
        void set_previous_position(const glm::vec3 & pos) { prevPos = pos; }

        virtual void update(float dt, const glm::vec3 & playerPos, const Maze & maze);
        virtual void draw() const;

        void set_collision_size(float halfWidth, float halfDepth)
        {
            collisionHalfWidth_ = halfWidth;
            collisionHalfDepth_ = halfDepth;
        }

    protected:
        glm::vec3 prevPos{ 0.0f };
        EnemyType type_;
        float pathSpeed_  = 0.0f;
        float chaseSpeed_ = 0.0f;
        float collisionHalfWidth_ = 0.5f;
        float collisionHalfDepth_ = 0.5f;
        std::vector<glm::ivec2> path_;
        glm::ivec2 lastPlayerTile_{ -1, -1 };
        glm::ivec2 targetTile_{ 0, 0 };
        std::size_t pathIndex_ = 0;

    private:
        float fireCooldown_{0.0f};
    };

    struct EnemySpawnWeights
    {
        float cylinderBot     = 1.0f;
        float sphereDrone     = 1.0f;
        float cubeTurret      = 1.0f;
        float pyramidCharger  = 1.0f;
    };

    class CylinderBot : public Enemy
    {
    public:
        CylinderBot();
    };

    class SphereDrone : public Enemy
    {
    public:
        SphereDrone();
    };

    class CubeTurret : public Enemy
    {
    public:
        CubeTurret();
    };

    class PyramidCharger : public Enemy
    {
    public:
        PyramidCharger();
    };

    std::vector<Enemy> & active_enemies();
    void spawn_enemies(const Maze & maze,
                       float tileScale,
                       const glm::ivec2 & playerStartCell,
                       const EnemySpawnWeights & weights);
    void update_enemies(float dt, game::PlayerMovement & playerState, const Maze & maze);
    void draw_enemies(const std::vector<Enemy> & enemies);
}

#endif // ENEMY_H
