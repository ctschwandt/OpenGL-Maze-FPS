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
#include "Maze.h"
#include "Player.h"

namespace game
{
    Enemy::Enemy(EnemyType type, float pathSpeed, float chaseSpeed, int baseHealth)
        : type_(type)
    {
        pathSpeed_  = pathSpeed;
        chaseSpeed_ = chaseSpeed;
        health      = baseHealth;
    }

    CylinderBot::CylinderBot()
        : Enemy(EnemyType::CylinderBot, 7.0f, 14.0f, 80)
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
        constexpr float TILE_OFFSET      = 0.5f;
        constexpr float TILE_SCALE       = 15.0f;
        constexpr float ENEMY_ROOM_RATIO = 1.0f; // 1 enemy per open tile (total), excluding player tile
        constexpr int   MAX_PATH_LENGTH  = 9;
        constexpr float CYLINDER_BOT_CONTACT_DAMAGE_PER_SECOND = 10.0f;

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
            return EnemyType::CylinderBot;
            std::array<std::pair<EnemyType, float>, 4> weightedTypes =
            {{
                { EnemyType::CylinderBot,    weights.cylinderBot },
                { EnemyType::SphereDrone,    weights.sphereDrone },
                { EnemyType::CubeTurret,     weights.cubeTurret },
                { EnemyType::PyramidCharger, weights.pyramidCharger }
            }};

            float totalWeight = std::accumulate(
                weightedTypes.begin(), weightedTypes.end(), 0.0f,
                [](float acc, const auto & entry)
                {
                    return acc + std::max(entry.second, 0.0f);
                });

            if (totalWeight <= 0.0f)
                return EnemyType::CylinderBot;

            float roll   = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
            float target = roll * totalWeight;

            float accumulated = 0.0f;
            for (const auto & entry : weightedTypes)
            {
                accumulated += std::max(entry.second, 0.0f);
                if (target <= accumulated)
                    return entry.first;
            }

            return weightedTypes.back().first;
        }

        bool collides_with_wall(float worldX,
                                float worldZ,
                                float collisionRadius,
                                const Maze & maze)
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
        }

        void resolve_enemy_collisions(std::vector<Enemy> & enemies, const Maze & maze)
        {
            constexpr float MIN_DIST_SQ_EPSILON = 0.0001f;

            const std::size_t count = enemies.size();
            for (std::size_t i = 0; i < count; ++i)
            {
                for (std::size_t j = i + 1; j < count; ++j)
                {
                    glm::vec2 diff(
                        enemies[j].pos.x - enemies[i].pos.x,
                        enemies[j].pos.z - enemies[i].pos.z);

                    float dist2 = glm::dot(diff, diff);
                    float minDist = enemies[i].radius + enemies[j].radius;
                    float minDist2 = minDist * minDist;

                    if (dist2 < minDist2 && dist2 > MIN_DIST_SQ_EPSILON)
                    {
                        float dist    = std::sqrt(dist2);
                        float overlap = minDist - dist;
                        glm::vec2 norm   = diff / dist;
                        glm::vec2 offset = norm * (overlap * 0.5f);

                        glm::vec3 posI    = enemies[i].pos;
                        glm::vec3 posJ    = enemies[j].pos;
                        glm::vec3 newPosI = posI;
                        glm::vec3 newPosJ = posJ;

                        newPosI.x -= offset.x;
                        newPosI.z -= offset.y;
                        newPosJ.x += offset.x;
                        newPosJ.z += offset.y;

                        bool okI = !collides_with_wall(newPosI.x, newPosI.z, enemies[i].radius, maze);
                        bool okJ = !collides_with_wall(newPosJ.x, newPosJ.z, enemies[j].radius, maze);

                        if (okI)
                            enemies[i].pos = newPosI;

                        if (okJ)
                            enemies[j].pos = newPosJ;
                    }
                }
            }
        }

        void apply_cylinder_bot_damage(game::PlayerMovement & playerState,
                                       const std::vector<Enemy> & enemies,
                                       float dt)
        {
            float contactRadius = game::PLAYER_RADIUS;
            float accumulatedDamage = 0.0f;

            for (const auto & enemy : enemies)
            {
                if (enemy.type() != EnemyType::CylinderBot)
                    continue;

                glm::vec2 diff(enemy.pos.x - playerState.position.x,
                               enemy.pos.z - playerState.position.z);

                float contactDistance = enemy.radius + contactRadius;
                if (glm::dot(diff, diff) <= contactDistance * contactDistance)
                {
                    accumulatedDamage += CYLINDER_BOT_CONTACT_DAMAGE_PER_SECOND * dt;
                }
            }

            playerState.damageBuffer += accumulatedDamage;
            int damage = static_cast<int>(playerState.damageBuffer);

            if (damage > 0)
            {
                playerState.health = std::max(0, playerState.health - damage);
                playerState.damageBuffer -= static_cast<float>(damage);
            }
        }
    } // anonymous namespace

    void Enemy::update(float dt, const glm::vec3 & playerPos, const Maze & maze)
    {
        switch (type_)
        {
        case EnemyType::CylinderBot:
        {
            constexpr float GROUND_Y   = 0.0f;
            constexpr float AGGRESSIVE_RANGE = TILE_SCALE;

            // CylinderBot stays on the ground plane.
            pos.y = GROUND_Y;

            glm::vec3 toPlayer = playerPos - pos;
            toPlayer.y         = 0.0f;
            float distanceToPlayer = glm::length(toPlayer);

            int curTr    = static_cast<int>(std::floor(pos.z / TILE_SCALE));
            int curTc    = static_cast<int>(std::floor(pos.x / TILE_SCALE));
            int playerTr = static_cast<int>(std::floor(playerPos.z / TILE_SCALE));
            int playerTc = static_cast<int>(std::floor(playerPos.x / TILE_SCALE));
            glm::ivec2 playerTile(playerTr, playerTc);

            if (distanceToPlayer < AGGRESSIVE_RANGE)
            {
                path_.clear();
                pathIndex_      = 0;
                lastPlayerTile_ = { -1, -1 };

                glm::vec3 dir = toPlayer;
                if (glm::length2(dir) > 0.0001f)
                    dir = glm::normalize(dir);

                vel = dir * chaseSpeed_;
            }
            else
            {
                bool reachedTargetTile = (glm::ivec2(curTr, curTc) == targetTile_);
                if (path_.empty() || playerTile != lastPlayerTile_ || reachedTargetTile)
                {
                    std::vector<glm::ivec2> newPath;
                    bool hasPath       = maze.findPath(curTr, curTc, playerTr, playerTc, newPath);
                    bool pathShortEnough = hasPath && static_cast<int>(newPath.size()) <= MAX_PATH_LENGTH;
                    if (pathShortEnough)
                    {
                        path_           = newPath;
                        lastPlayerTile_ = playerTile;
                        if (path_.size() > 1)
                        {
                            pathIndex_  = 1;
                            targetTile_ = path_[pathIndex_];
                        }
                        else
                        {
                            pathIndex_  = 0;
                            targetTile_ = playerTile;
                        }
                    }
                    else
                    {
                        // Without a valid path, or if the path is too long, fall back
                        // to staying put instead of marching straight toward the player
                        // (which can ignore walls). Keep attempting to find a shorter
                        // path on subsequent updates.
                        path_.clear();
                        pathIndex_      = 0;
                        targetTile_     = { curTr, curTc };
                        lastPlayerTile_ = { -1, -1 };
                        vel             = glm::vec3(0.0f);
                        return;
                    }
                }

                float targetCenterX = (static_cast<float>(targetTile_.y) + 0.5f) * TILE_SCALE;
                float targetCenterZ = (static_cast<float>(targetTile_.x) + 0.5f) * TILE_SCALE;
                glm::vec3 targetPos(targetCenterX, GROUND_Y, targetCenterZ);
                glm::vec3 dir       = targetPos - pos;
                dir.y               = 0.0f;
                if (glm::length2(dir) > 0.0001f)
                    dir = glm::normalize(dir);

                vel = dir * pathSpeed_;
            }

            glm::vec3 newPos = pos + vel * dt;

            if (collides_with_wall(newPos.x, pos.z, collisionHalfWidth_, maze))
            {
                newPos.x = pos.x;
                vel.x    = 0.0f;
            }

            if (collides_with_wall(newPos.x, newPos.z, collisionHalfWidth_, maze))
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
                glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT);
                glDisable(GL_LIGHTING);
                glDisable(GL_CULL_FACE);

                glPushMatrix();
                glTranslatef(pos.x, pos.y + (height * 0.5f), pos.z);
                glRotatef(yaw * 180.0f / static_cast<float>(M_PI), 0.0f, 1.0f, 0.0f);

                glColor3f(0.2f, 0.6f, 1.0f);
                draw_cylinder(radius, height, 24);

                glPopMatrix();

                glPopAttrib();
                break;
            }

        case EnemyType::SphereDrone:
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
                       float tileScale,
                       const glm::ivec2 & playerStartCell, // (room_r, room_c) in logical maze coords
                       const EnemySpawnWeights & weights)
    {    
        auto & enemies = active_enemies();
        enemies.clear();

        const int tileN = maze.tiles_n;      // = 2 * n + 1

        // Convert player logical cell -> tile indices
        const int playerTileR = playerStartCell.x * 2 + 1;
        const int playerTileC = playerStartCell.y * 2 + 1;

        // Collect all open tiles (non-wall) except the player's tile.
        std::vector<glm::ivec2> openTiles;
        openTiles.reserve(tileN * tileN);

        for (int tr = 0; tr < tileN; ++tr)
        {
            for (int tc = 0; tc < tileN; ++tc)
            {
                if (maze.is_wall_tile(tr, tc))
                    continue;

                if (tr == playerTileR && tc == playerTileC)
                    continue; // don't spawn on the player

                openTiles.emplace_back(tr, tc);
            }
        }

        if (openTiles.empty())
            return;

        int openCount  = static_cast<int>(openTiles.size());
        int spawnCount = static_cast<int>(openCount * ENEMY_ROOM_RATIO);

        //std::cout << spawnCount << std::endl;
        
        for (int i = 0; i < spawnCount; ++i)
        {
            // Pick a random open tile WITH replacement.
            int choice = std::rand() % openTiles.size();
            glm::ivec2 tile = openTiles[choice]; // (tr, tc)

            Enemy enemy = make_enemy(pick_enemy_type(weights));

            // Center of that tile in world space.
            enemy.pos = glm::vec3(
                (static_cast<float>(tile.y) + TILE_OFFSET) * tileScale, // x
                0.0f,                                                   // y
                (static_cast<float>(tile.x) + TILE_OFFSET) * tileScale  // z
            );

            enemies.push_back(enemy);
        }
    }

    void update_enemies(float dt, game::PlayerMovement & playerState, const Maze & maze)
    {
        const glm::vec3 playerPos = playerState.position;
        auto & enemies = active_enemies();
        for (auto & enemy : enemies)
            enemy.set_previous_position(enemy.pos);

        for (auto & enemy : enemies)
            enemy.update(dt, playerPos, maze);

        resolve_enemy_collisions(enemies, maze);

        for (auto & enemy : enemies)
        {
            if (collides_with_wall(enemy.pos.x, enemy.pos.z, enemy.radius, maze))
            {
                enemy.pos = enemy.previous_position();
                enemy.vel = glm::vec3(0.0f);
            }
        }

        apply_cylinder_bot_damage(playerState, enemies, dt);
    }

    void draw_enemies(const std::vector<Enemy> & enemies)
    {
        for (const auto & enemy : enemies)
            enemy.draw();
    }
}
