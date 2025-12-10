#pragma once
#include <glm/vec2.hpp>

class Maze;

namespace gameflow
{
    extern Maze maze;
    extern const float TILE_SCALE;

    void start_new_run(bool resetPlayerStats = true);
    bool maze_had_enemies();
    void set_maze_had_enemies(bool value);
}
