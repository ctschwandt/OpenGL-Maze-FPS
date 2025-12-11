#include "Enemy.h"

#include <GL/glew.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <numeric>
#include <vector>

#include <glm/gtx/norm.hpp>

#include "Draw.h"
#include "Globals.h"
#include "Maze.h"
#include "Player.h"
#include "Projectile.h"

namespace game
{
    Enemy::Enemy(EnemyType type, float path_speed, float chase_speed, int base_health)
        : type_(type)
    {
        path_speed_ = path_speed;
        chase_speed_ = chase_speed;
        health = base_health;
        fire_cooldown_ = 0.0f;
    }

    EnemyType Enemy::type() const
    {
        return type_;
    }

    float Enemy::path_speed() const
    {
        return path_speed_;
    }

    float Enemy::chase_speed() const
    {
        return chase_speed_;
    }

    glm::vec3 Enemy::previous_position() const
    {
        return prev_pos;
    }

    void Enemy::set_previous_position(const glm::vec3 & pos)
    {
        prev_pos = pos;
    }

    void Enemy::set_collision_size(float half_width, float half_depth)
    {
        collision_half_width_ = half_width;
        collision_half_depth_ = half_depth;
    }

    CylinderBot::CylinderBot()
        : Enemy(EnemyType::CylinderBot, 5.0f, 6.0f, 80)
    {
        radius = 0.8f;
        height = 2.4f;
        set_collision_size(radius, radius);
    }

    SphereDrone::SphereDrone()
        : Enemy(EnemyType::SphereDrone, 6.0f, 8.0f, 60)
    {
        radius = 0.7f;
        height = 1.0f;
        set_collision_size(radius, radius);
    }

    CubeTurret::CubeTurret()
        : Enemy(EnemyType::CubeTurret, 0.0f, 0.0f, 120)
    {
        radius = 0.9f;
        height = 1.2f;
        set_collision_size(radius, radius);
    }

    PyramidCharger::PyramidCharger()
        : Enemy(EnemyType::PyramidCharger, 7.0f, 10.0f, 90)
    {
        radius = 0.8f;
        height = 1.4f;
        set_collision_size(radius, radius);
    }

    namespace
    {
        const float TILE_OFFSET = 0.5f;
        const float TILE_SCALE = 15.0f;
        const float ENEMY_ROOM_RATIO = 0.6f;
        const int MAX_PATH_LENGTH = 9;
        const float CYLINDER_BOT_CONTACT_DAMAGE_PER_SECOND = 10.0f;
        const float SPHERE_DRONE_HOVER_Y = 1.5f;
        const float SPHERE_DRONE_FIRE_RANGE = TILE_SCALE * 2.5f;
        const float SPHERE_DRONE_DESIRED_RANGE = SPHERE_DRONE_FIRE_RANGE * 0.8f;
        const float SPHERE_DRONE_FIRE_COOLDOWN = 1.5f;
        const float SPHERE_DRONE_PROJECTILE_SPEED = 10.0f;
        const int SPHERE_DRONE_PROJECTILE_DAMAGE = 10;

        Enemy make_enemy(EnemyType type)
        {
            switch (type)
            {
            case EnemyType::CylinderBot:
                return CylinderBot();
            case EnemyType::SphereDrone:
                return SphereDrone();
            case EnemyType::CubeTurret:
                return CubeTurret();
            case EnemyType::PyramidCharger:
            default:
                return PyramidCharger();
            }
        }

        EnemyType pick_enemy_type(const EnemySpawnWeights & weights)
        {
            std::array<std::pair<EnemyType, float>, 2> weighted_types =
                {{{EnemyType::CylinderBot, weights.cylinder_bot},
                  {EnemyType::SphereDrone, weights.sphere_drone}}};

            float total_weight = std::accumulate(
                weighted_types.begin(), weighted_types.end(), 0.0f,
                [](float acc, const auto & entry)
                {
                    return acc + std::max(entry.second, 0.0f);
                });

            if (total_weight <= 0.0f)
                return EnemyType::CylinderBot;

            float roll_value = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
            float target = roll_value * total_weight;

            float accumulated = 0.0f;
            for (const auto & entry : weighted_types)
            {
                accumulated += std::max(entry.second, 0.0f);
                if (target <= accumulated)
                    return entry.first;
            }

            return weighted_types.back().first;
        }

        bool collides_with_wall(float world_x,
                                float world_z,
                                float collision_radius,
                                const Maze & maze)
        {
            int x0 = static_cast<int>(std::floor((world_x - collision_radius) / TILE_SCALE));
            int x1 = static_cast<int>(std::floor((world_x + collision_radius) / TILE_SCALE));
            int z0 = static_cast<int>(std::floor((world_z - collision_radius) / TILE_SCALE));
            int z1 = static_cast<int>(std::floor((world_z + collision_radius) / TILE_SCALE));

            for (int tr = z0; tr <= z1; ++tr)
            {
                for (int tc = x0; tc <= x1; ++tc)
                {
                    if (maze.is_wall_tile(tr, tc))
                        return true;
                }
            }

            return false;
        }

        bool collides_with_player(float world_x,
                                  float world_z,
                                  float actor_radius,
                                  const glm::vec3 & player_pos,
                                  float player_radius)
        {
            float dx = world_x - player_pos.x;
            float dz = world_z - player_pos.z;
            float min_dist = actor_radius + player_radius;
            return (dx * dx + dz * dz) < (min_dist * min_dist);
        }

        void resolve_enemy_collisions(std::vector<Enemy> & enemies, const Maze & maze)
        {
            const float MIN_DIST_SQ_EPSILON = 0.0001f;

            const std::size_t count = enemies.size();
            for (std::size_t i = 0; i < count; ++i)
            {
                for (std::size_t j = i + 1; j < count; ++j)
                {
                    glm::vec2 diff(
                        enemies[j].pos.x - enemies[i].pos.x,
                        enemies[j].pos.z - enemies[i].pos.z);

                    float dist2 = glm::dot(diff, diff);
                    float min_dist = enemies[i].radius + enemies[j].radius;
                    float min_dist2 = min_dist * min_dist;

                    if (dist2 < min_dist2 && dist2 > MIN_DIST_SQ_EPSILON)
                    {
                        float dist = std::sqrt(dist2);
                        float overlap_amount = min_dist - dist;
                        glm::vec2 norm = diff / dist;
                        glm::vec2 offset = norm * (overlap_amount * 0.5f);

                        glm::vec3 pos_i = enemies[i].pos;
                        glm::vec3 pos_j = enemies[j].pos;
                        glm::vec3 new_pos_i = pos_i;
                        glm::vec3 new_pos_j = pos_j;

                        new_pos_i.x -= offset.x;
                        new_pos_i.z -= offset.y;
                        new_pos_j.x += offset.x;
                        new_pos_j.z += offset.y;

                        bool ok_i = !collides_with_wall(new_pos_i.x, new_pos_i.z, enemies[i].radius, maze);
                        bool ok_j = !collides_with_wall(new_pos_j.x, new_pos_j.z, enemies[j].radius, maze);

                        if (ok_i)
                            enemies[i].pos = new_pos_i;

                        if (ok_j)
                            enemies[j].pos = new_pos_j;
                    }
                }
            }
        }

        void apply_cylinder_bot_damage(game::PlayerMovement & player_state,
                                       const std::vector<Enemy> & enemies,
                                       float dt)
        {
            float contact_radius = game::PLAYER_RADIUS;
            float accumulated_damage = 0.0f;

            for (const auto & enemy : enemies)
            {
                if (enemy.type() != EnemyType::CylinderBot)
                    continue;

                glm::vec2 diff(enemy.pos.x - player_state.position.x,
                               enemy.pos.z - player_state.position.z);

                float contact_distance = enemy.radius + contact_radius;
                if (glm::dot(diff, diff) <= contact_distance * contact_distance)
                {
                    accumulated_damage += CYLINDER_BOT_CONTACT_DAMAGE_PER_SECOND * dt;
                }
            }

            player_state.damageBuffer += accumulated_damage;
            int damage = static_cast<int>(player_state.damageBuffer);

            if (damage > 0)
            {
                player_state.health = std::max(0, player_state.health - damage);
                player_state.damageBuffer -= static_cast<float>(damage);
            }
        }
    } // anonymous namespace

    void Enemy::update(float dt, const glm::vec3 & player_pos, float player_radius, const Maze & maze)
    {
        switch (type_)
        {
        case EnemyType::CylinderBot:
        {
            const float GROUND_Y = 0.0f;
            const float AGGRESSIVE_RANGE = TILE_SCALE;

            // CylinderBot stays on the ground plane.
            pos.y = GROUND_Y;

            glm::vec3 to_player = player_pos - pos;
            to_player.y = 0.0f;
            float distance_to_player = glm::length(to_player);

            int cur_tr = static_cast<int>(std::floor(pos.z / TILE_SCALE));
            int cur_tc = static_cast<int>(std::floor(pos.x / TILE_SCALE));
            int player_tr = static_cast<int>(std::floor(player_pos.z / TILE_SCALE));
            int player_tc = static_cast<int>(std::floor(player_pos.x / TILE_SCALE));
            glm::ivec2 player_tile(player_tr, player_tc);

            if (distance_to_player < AGGRESSIVE_RANGE)
            {
                path_.clear();
                path_index_ = 0;
                last_player_tile_ = { -1, -1 };

                glm::vec3 dir = to_player;
                if (glm::length2(dir) > 0.0001f)
                    dir = glm::normalize(dir);

                vel = dir * chase_speed_;
            }
            else
            {
                bool reached_target_tile = (glm::ivec2(cur_tr, cur_tc) == target_tile_);
                if (path_.empty() || player_tile != last_player_tile_ || reached_target_tile)
                {
                    std::vector<glm::ivec2> new_path;
                    bool has_path = maze.findPath(cur_tr, cur_tc, player_tr, player_tc, new_path);
                    bool path_short_enough = has_path && static_cast<int>(new_path.size()) <= MAX_PATH_LENGTH;
                    if (path_short_enough)
                    {
                        path_ = new_path;
                        last_player_tile_ = player_tile;
                        if (path_.size() > 1)
                        {
                            path_index_ = 1;
                            target_tile_ = path_[path_index_];
                        }
                        else
                        {
                            path_index_ = 0;
                            target_tile_ = player_tile;
                        }
                    }
                    else
                    {
                        // Without a valid path, or if the path is too long, fall back
                        // to staying put instead of marching straight toward the player
                        // (which can ignore walls). Keep attempting to find a shorter
                        // path on subsequent updates.
                        path_.clear();
                        path_index_ = 0;
                        target_tile_ = { cur_tr, cur_tc };
                        last_player_tile_ = { -1, -1 };
                        vel = glm::vec3(0.0f);
                        return;
                    }
                }

                float target_center_x = (static_cast<float>(target_tile_.y) + 0.5f) * TILE_SCALE;
                float target_center_z = (static_cast<float>(target_tile_.x) + 0.5f) * TILE_SCALE;
                glm::vec3 target_pos(target_center_x, GROUND_Y, target_center_z);
                glm::vec3 dir = target_pos - pos;
                dir.y = 0.0f;
                if (glm::length2(dir) > 0.0001f)
                    dir = glm::normalize(dir);

                vel = dir * path_speed_;
            }

            glm::vec3 new_pos = pos + vel * dt;

            if (collides_with_wall(new_pos.x, pos.z, collision_half_width_, maze))
            {
                new_pos.x = pos.x;
                vel.x = 0.0f;
            }

            if (collides_with_wall(new_pos.x, new_pos.z, collision_half_width_, maze))
            {
                new_pos.z = pos.z;
                vel.z = 0.0f;
            }

            if (collides_with_player(new_pos.x, pos.z, radius, player_pos, player_radius))
            {
                new_pos.x = pos.x;
                vel.x = 0.0f;
            }

            if (collides_with_player(new_pos.x, new_pos.z, radius, player_pos, player_radius))
            {
                new_pos.z = pos.z;
                vel.z = 0.0f;
            }

            pos = new_pos;
            pos.y = GROUND_Y;

            yaw = std::atan2(player_pos.x - pos.x, player_pos.z - pos.z) + M_PI / 2;
            break;
        }

        case EnemyType::SphereDrone:
        {
            const float AGGRESSIVE_RANGE = TILE_SCALE;
            pos.y = SPHERE_DRONE_HOVER_Y;

            glm::vec3 to_player = player_pos - pos;
            glm::vec3 to_player_horiz(to_player.x, 0.0f, to_player.z);
            float distance_horiz = glm::length(to_player_horiz);

            int cur_tr = static_cast<int>(std::floor(pos.z / TILE_SCALE));
            int cur_tc = static_cast<int>(std::floor(pos.x / TILE_SCALE));
            int player_tr = static_cast<int>(std::floor(player_pos.z / TILE_SCALE));
            int player_tc = static_cast<int>(std::floor(player_pos.x / TILE_SCALE));
            glm::ivec2 player_tile(player_tr, player_tc);

            // NEW: maintain a stand-off distance once in desired range
            if (distance_horiz <= SPHERE_DRONE_DESIRED_RANGE)
            {
                // In firing range: hover here (no forward movement)
                vel = glm::vec3(0.0f);
            }
            else
            {
                if (distance_horiz < AGGRESSIVE_RANGE)
                {
                    path_.clear();
                    path_index_ = 0;
                    last_player_tile_ = { -1, -1 };

                    glm::vec3 dir = (distance_horiz > 0.0001f)
                                        ? to_player_horiz / distance_horiz
                                        : glm::vec3(0.0f, 0.0f, 0.0f);

                    vel = dir * chase_speed_;
                }
                else
                {
                    bool reached_target_tile = (glm::ivec2(cur_tr, cur_tc) == target_tile_);
                    if (path_.empty() || player_tile != last_player_tile_ || reached_target_tile)
                    {
                        std::vector<glm::ivec2> new_path;
                        bool has_path = maze.findPath(cur_tr, cur_tc, player_tr, player_tc, new_path);
                        bool path_short_enough = has_path && static_cast<int>(new_path.size()) <= MAX_PATH_LENGTH;
                        if (path_short_enough)
                        {
                            path_ = new_path;
                            last_player_tile_ = player_tile;
                            if (path_.size() > 1)
                            {
                                path_index_ = 1;
                                target_tile_ = path_[path_index_];
                            }
                            else
                            {
                                path_index_ = 0;
                                target_tile_ = player_tile;
                            }
                        }
                        else
                        {
                            path_.clear();
                            path_index_ = 0;
                            target_tile_ = { cur_tr, cur_tc };
                            last_player_tile_ = { -1, -1 };
                            vel = glm::vec3(0.0f);
                            pos.y = SPHERE_DRONE_HOVER_Y;
                            return;
                        }
                    }

                    float target_center_x = (static_cast<float>(target_tile_.y) + 0.5f) * TILE_SCALE;
                    float target_center_z = (static_cast<float>(target_tile_.x) + 0.5f) * TILE_SCALE;
                    glm::vec3 target_pos(target_center_x, SPHERE_DRONE_HOVER_Y, target_center_z);
                    glm::vec3 dir = target_pos - pos;
                    dir.y = 0.0f;
                    if (glm::length2(dir) > 0.0001f)
                        dir = glm::normalize(dir);

                    vel = dir * path_speed_;
                }
            }

            glm::vec3 new_pos = pos + vel * dt;

            if (collides_with_wall(new_pos.x, pos.z, collision_half_width_, maze))
            {
                new_pos.x = pos.x;
                vel.x = 0.0f;
            }

            if (collides_with_wall(new_pos.x, new_pos.z, collision_half_width_, maze))
            {
                new_pos.z = pos.z;
                vel.z = 0.0f;
            }

            if (collides_with_player(new_pos.x, pos.z, radius, player_pos, player_radius))
            {
                new_pos.x = pos.x;
                vel.x = 0.0f;
            }

            if (collides_with_player(new_pos.x, new_pos.z, radius, player_pos, player_radius))
            {
                new_pos.z = pos.z;
                vel.z = 0.0f;
            }

            pos = new_pos;
            pos.y = SPHERE_DRONE_HOVER_Y;

            yaw = std::atan2(player_pos.x - pos.x, player_pos.z - pos.z) + M_PI / 2;

            fire_cooldown_ = std::max(0.0f, fire_cooldown_ - dt);
            float distance_to_player = glm::length(to_player);

            if (fire_cooldown_ <= 0.0f && distance_to_player < SPHERE_DRONE_FIRE_RANGE)
            {
                glm::vec3 fire_dir = (distance_to_player > 0.0001f)
                                        ? (to_player / distance_to_player)
                                        : glm::vec3(1.0f, 0.0f, 0.0f);

                Projectile shot;
                shot.fromPlayer = false;
                shot.damage = SPHERE_DRONE_PROJECTILE_DAMAGE;
                shot.position = pos + fire_dir * (radius * 0.8f);
                shot.velocity = fire_dir * SPHERE_DRONE_PROJECTILE_SPEED;

                active_projectiles().push_back(shot);
                fire_cooldown_ = SPHERE_DRONE_FIRE_COOLDOWN;
            }

            break;
        }

        case EnemyType::CubeTurret:
        case EnemyType::PyramidCharger:
        default:
            // TODO: implement behavior for other enemy types
            break;
        }
    }

    void Enemy::draw() const
    {
        switch (type_)
        {
        case EnemyType::CylinderBot:
        {
            glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT);

            glDisable(GL_LIGHTING);
            glDisable(GL_CULL_FACE);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, globals::landon_texture);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

            glPushMatrix();
            glTranslatef(pos.x, pos.y + (height * 0.5f), pos.z);
            glRotatef(yaw * 180.0f / static_cast<float>(M_PI), 0.0f, 1.0f, 0.0f);

            glColor3f(1.0f, 1.0f, 1.0f);
            draw_textured_cylinder(radius, height, 24);

            glPopMatrix();

            glBindTexture(GL_TEXTURE_2D, 0);
            glPopAttrib();
            break;
        }

        case EnemyType::SphereDrone:
        {
            glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT);

            glDisable(GL_LIGHTING);
            glDisable(GL_CULL_FACE);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, globals::liow_texture);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

            glMatrixMode(GL_TEXTURE);
            glPushMatrix();
            glLoadIdentity();
            glScalef(1.0f, -1.0f, 1.0f);
            glTranslatef(0.0f, -1.0f, 0.0f);
            glMatrixMode(GL_MODELVIEW);

            glPushMatrix();
            glTranslatef(pos.x, pos.y, pos.z);
            float yaw_degrees = yaw * 180.0f / M_PI;
            glRotatef(yaw_degrees, 0.0f, 1.0f, 0.0f);
            glColor3f(1.0f, 1.0f, 1.0f);
            draw_sphere(radius, 20, 32);
            glPopMatrix();

            glMatrixMode(GL_TEXTURE);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);

            glBindTexture(GL_TEXTURE_2D, 0);
            glPopAttrib();
            break;
        }

        case EnemyType::CubeTurret:
        case EnemyType::PyramidCharger:
        default:
            // TODO: draw other enemy types
            break;
        }
    }

    std::vector<Enemy> & active_enemies()
    {
        static std::vector<Enemy> enemies;
        return enemies;
    }

    void spawn_enemies(const Maze & maze,
                       float tile_scale,
                       const glm::ivec2 & player_start_cell, // (room_r, room_c) in logical maze coords
                       const EnemySpawnWeights & weights)
    {
        auto & enemies = active_enemies();
        enemies.clear();

        const int tile_n = maze.tiles_n; // = 2 * n + 1

        // Convert player logical cell -> tile indices
        const int player_tile_r = player_start_cell.x * 2 + 1;
        const int player_tile_c = player_start_cell.y * 2 + 1;

        // Collect all open tiles (non-wall) except the player's tile.
        std::vector<glm::ivec2> open_tiles;
        open_tiles.reserve(tile_n * tile_n);

        for (int tr = 0; tr < tile_n; ++tr)
        {
            for (int tc = 0; tc < tile_n; ++tc)
            {
                if (maze.is_wall_tile(tr, tc))
                    continue;

                if (tr == player_tile_r && tc == player_tile_c)
                    continue; // don't spawn on the player

                open_tiles.emplace_back(tr, tc);
            }
        }

        if (open_tiles.empty())
            return;

        int open_count = static_cast<int>(open_tiles.size());
        int spawn_count = static_cast<int>(open_count * ENEMY_ROOM_RATIO);

        // Never spawn more enemies than there are open tiles.
        if (spawn_count > open_count)
            spawn_count = open_count;

        // Sample tiles WITHOUT replacement using a partial Fisher–Yates shuffle.
        for (int i = 0; i < spawn_count; ++i)
        {
            // Number of remaining tiles we haven't "fixed" yet
            int remaining = open_count - i;
            int choice_offset = std::rand() % remaining;
            int choice_index = i + choice_offset;

            // Bring the chosen tile into position i
            std::swap(open_tiles[i], open_tiles[choice_index]);
            glm::ivec2 tile = open_tiles[i]; // (tr, tc)

            Enemy enemy = make_enemy(pick_enemy_type(weights));

            // Center of that tile in world space.
            enemy.pos = glm::vec3(
                (static_cast<float>(tile.y) + TILE_OFFSET) * tile_scale, // x
                0.0f,                                                   // y
                (static_cast<float>(tile.x) + TILE_OFFSET) * tile_scale  // z
            );

            enemies.push_back(enemy);
        }
    }

    void update_enemies(float dt, game::PlayerMovement & player_state, const Maze & maze)
    {
        const glm::vec3 player_pos = player_state.position;
        auto & enemies = active_enemies();

        if (globals::enemy_freeze_active)
        {
            for (auto & enemy : enemies)
            {
                enemy.set_previous_position(enemy.pos);
                enemy.vel = glm::vec3(0.0f);
            }
            return;
        }

        for (auto & enemy : enemies)
            enemy.set_previous_position(enemy.pos);

        for (auto & enemy : enemies)
            enemy.update(dt, player_pos, player_state.collisionRadius, maze);

        resolve_enemy_collisions(enemies, maze);

        for (auto & enemy : enemies)
        {
            if (collides_with_wall(enemy.pos.x, enemy.pos.z, enemy.radius, maze))
            {
                enemy.pos = enemy.previous_position();
                enemy.vel = glm::vec3(0.0f);
            }
        }

        apply_cylinder_bot_damage(player_state, enemies, dt);
    }

    void draw_enemies(const std::vector<Enemy> & enemies)
    {
        for (const auto & enemy : enemies)
            enemy.draw();
    }
} // namespace game
