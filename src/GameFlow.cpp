#include "GameFlow.h"

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#include <glm/vec2.hpp>

#include "Enemy.h"
#include "AppInit.h"
#include "Globals.h"
#include "Maze.h"
#include "Player.h"
#include "Projectile.h"
#include "mygllib/SingletonView.h"
#include "mygllib/View.h"

namespace gameflow
{
    namespace
    {
        const int MIN_MAZE_N = 3;
        const int MAX_MAZE_N = 8;

        const game::EnemySpawnWeights ENEMY_SPAWN_WEIGHTS{ 1.0f, 1.0f, 1.0f, 1.0f };

        bool g_maze_had_enemies = false;

        glm::ivec2 random_start_cell(int maze_n)
        {
            int start_r = std::rand() % maze_n;
            int start_c = std::rand() % maze_n;
            return glm::ivec2(start_r, start_c);
        }

        void place_player_at_cell(const glm::ivec2 & cell)
        {
            float start_x = TILE_SCALE * (2.0f * static_cast<float>(cell.y) + 1.5f);
            float start_z = TILE_SCALE * (2.0f * static_cast<float>(cell.x) + 1.5f);

            mygllib::View & view = *(mygllib::SingletonView::getInstance());
            float yaw = static_cast<float>(view.yaw());
            float eyeOffsetX = std::cos(yaw) * game::PLAYER_RADIUS;
            float eyeOffsetZ = std::sin(yaw) * game::PLAYER_RADIUS;
            view.eye(start_x + eyeOffsetX, game::PLAYER_EYE_HEIGHT, start_z + eyeOffsetZ);
            view.update_center_from_yaw_pitch();
        }

        void reset_player_state_for_spawn(bool resetStats)
        {
            game::PlayerMovement & playerState = game::player_movement_state();
            const int preservedHealth    = playerState.health;
            const int preservedMaxHealth = playerState.max_health;
            const int preservedScore     = playerState.score;

            playerState = game::PlayerMovement();

            if (!resetStats)
            {
                playerState.health    = preservedHealth;
                playerState.max_health = preservedMaxHealth;
                playerState.score     = preservedScore;
            }
            playerState.initialized = false;
        }
    } // namespace

    Maze maze;
    const float TILE_SCALE = 15.0f;

    void start_new_run(bool resetPlayerStats)
    {
        mygllib::View & view = *(mygllib::SingletonView::getInstance());
        const float old_yaw = view.yaw();
        const float old_pitch = view.pitch();
        
        // 1) Choose a random maze size n in [MIN_MAZE_N, MAX_MAZE_N]
        int newMazeN = MIN_MAZE_N + (std::rand() % (MAX_MAZE_N - MIN_MAZE_N + 1));

        // 2) Rebuild the Maze if the size changed
        if (maze.n != newMazeN)
        {
            maze = Maze(newMazeN);
        }

        // 3) Random start cell for this maze size
        glm::ivec2 playerStartCell = random_start_cell(maze.n);

        globals::enemy_freeze_active        = false;
        globals::enemy_freeze_used_this_run = false;

        maze.init(playerStartCell.x, playerStartCell.y);

        place_player_at_cell(playerStartCell);
        view.yaw() = old_yaw;
        view.pitch() = old_pitch;
        view.update_center_from_yaw_pitch();
        reset_player_state_for_spawn(resetPlayerStats);
        game::active_projectiles().clear();
        game::spawn_enemies(maze, TILE_SCALE, playerStartCell, ENEMY_SPAWN_WEIGHTS);
        g_maze_had_enemies = !game::active_enemies().empty();
        appinit::init_textures();
    }

    bool maze_had_enemies()
    {
        return g_maze_had_enemies;
    }

    void set_maze_had_enemies(bool value)
    {
        g_maze_had_enemies = value;
    }

} // namespace gameflow
