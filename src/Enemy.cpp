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

namespace game
{
    Enemy::Enemy(EnemyType type, float moveSpeed, int baseHealth)
        : type_(type)
    {
        moveSpeed_      = moveSpeed;
        health          = baseHealth;
    }

    RectBot::RectBot()
        : Enemy(EnemyType::RectBot, 14.0f, 80)
    {
        radius = 0.5f;
        height = 2.4f;
        set_collision_size(0.9f, 0.6f);
    }

    SphereDrone::SphereDrone()
        : Enemy(EnemyType::SphereDrone, 8.0f, 60)
    {
        radius = 0.7f;
        height = 1.0f;
        set_collision_size(radius, radius);
    }

    CubeTurret::CubeTurret()
        : Enemy(EnemyType::CubeTurret, 0.0f, 120)
    {
        radius = 0.9f;
        height = 1.2f;
        set_collision_size(radius, radius);
    }

    PyramidCharger::PyramidCharger()
        : Enemy(EnemyType::PyramidCharger, 10.0f, 90)
    {
        radius = 0.8f;
        height = 1.4f;
        set_collision_size(radius, radius);
    }

    namespace
    {
        constexpr float TILE_OFFSET      = 0.5f;
        constexpr float ENEMY_ROOM_RATIO = 1.0f; // 1 enemy per open tile (total), excluding player tile

        Enemy make_enemy(EnemyType type)
        {
            switch (type)
            {
            case EnemyType::RectBot:
                return RectBot();
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
            return EnemyType::RectBot;
            std::array<std::pair<EnemyType, float>, 4> weightedTypes =
            {{
                { EnemyType::RectBot,        weights.rectBot },
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
                return EnemyType::RectBot;

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
    } // anonymous namespace

    void Enemy::update(float dt, const glm::vec3 & playerPos, const Maze & maze)
    {
        switch (type_)
        {
        case EnemyType::RectBot:
        {
            constexpr float GROUND_Y   = 0.0f;
            constexpr float TILE_SCALE = 15.0f;
            constexpr float AGGRESSIVE_RANGE = TILE_SCALE;

            // RectBot stays on the ground plane.
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

                vel = dir * moveSpeed_;
            }
            else
            {
                bool reachedTargetTile = (glm::ivec2(curTr, curTc) == targetTile_);
                if (path_.empty() || playerTile != lastPlayerTile_ || reachedTargetTile)
                {
                    std::vector<glm::ivec2> newPath;
                    if (maze.findPath(curTr, curTc, playerTr, playerTc, newPath))
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
                        // Without a valid path, fall back to staying put instead of
                        // marching straight toward the player (which can ignore
                        // walls). Keep attempting to find a path on subsequent
                        // updates.
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

                vel = dir * moveSpeed_;
            }

            auto collides_with_wall = [&](float worldX, float worldZ, float halfWidth, float halfDepth) -> bool
            {
                int x0 = static_cast<int>(std::floor((worldX - halfWidth) / TILE_SCALE));
                int x1 = static_cast<int>(std::floor((worldX + halfWidth) / TILE_SCALE));
                int z0 = static_cast<int>(std::floor((worldZ - halfDepth) / TILE_SCALE));
                int z1 = static_cast<int>(std::floor((worldZ + halfDepth) / TILE_SCALE));

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

            if (collides_with_wall(newPos.x, pos.z, collisionHalfWidth_, collisionHalfDepth_))
            {
                newPos.x = pos.x;
                vel.x    = 0.0f;
            }

            if (collides_with_wall(newPos.x, newPos.z, collisionHalfWidth_, collisionHalfDepth_))
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
        case EnemyType::RectBot:
        {
            glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT);
            glDisable(GL_LIGHTING);

            glPushMatrix();
            glTranslatef(pos.x, pos.y + (height * 0.5f), pos.z);
            glRotatef(yaw * 180.0f / static_cast<float>(M_PI), 0.0f, 1.0f, 0.0f);

            glColor3f(0.2f, 0.6f, 1.0f);
            draw_box(collisionHalfWidth_ * 2.0f, height, collisionHalfDepth_ * 2.0f);

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
        const int n     = maze.n;

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

    void update_enemies(float dt, const glm::vec3 & playerPos, const Maze & maze)
    {
        auto & enemies = active_enemies();
        for (auto & enemy : enemies)
            enemy.update(dt, playerPos, maze);
    }

    void draw_enemies(const std::vector<Enemy> & enemies)
    {
        for (const auto & enemy : enemies)
            enemy.draw();
    }
}
