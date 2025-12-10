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
        Enemy(EnemyType type, float path_speed, float chase_speed, int base_health);
        virtual ~Enemy() = default;

        EnemyType type() const;
        float path_speed() const;
        float chase_speed() const;

        glm::vec3 previous_position() const;
        void set_previous_position(const glm::vec3 & pos);

        virtual void update(float dt, const glm::vec3 & player_pos, const Maze & maze);
        virtual void draw() const;

        void set_collision_size(float half_width, float half_depth);

    protected:
        glm::vec3 prev_pos{ 0.0f };
        EnemyType type_;
        float path_speed_ = 0.0f;
        float chase_speed_ = 0.0f;
        float collision_half_width_ = 0.5f;
        float collision_half_depth_ = 0.5f;
        std::vector<glm::ivec2> path_;
        glm::ivec2 last_player_tile_{ -1, -1 };
        glm::ivec2 target_tile_{ 0, 0 };
        std::size_t path_index_ = 0;

    private:
        float fire_cooldown_{ 0.0f };
    };

    struct EnemySpawnWeights
    {
        float cylinder_bot = 1.0f;
        float sphere_drone = 1.0f;
        float cube_turret = 1.0f;
        float pyramid_charger = 1.0f;
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
                       float tile_scale,
                       const glm::ivec2 & player_start_cell,
                       const EnemySpawnWeights & weights);
    void update_enemies(float dt, game::PlayerMovement & player_state, const Maze & maze);
    void draw_enemies(const std::vector<Enemy> & enemies);
} // namespace game

#endif // ENEMY_H
