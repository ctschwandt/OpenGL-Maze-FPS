#pragma once
#include <vector>

class Maze;
namespace game { struct PlayerMovement; struct Projectile; }

namespace render
{
    void draw_maze(const Maze & maze, float tileScale);
    void draw_player(const game::PlayerMovement & playerState);
    void draw_player_indicator(const game::PlayerMovement & playerState);
    void draw_projectiles(const Maze & maze,
                          float tileScale,
                          const std::vector<game::Projectile> & projectiles);
    void draw_robert_cube();
}
