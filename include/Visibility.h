#pragma once
#include <glm/vec3.hpp>

class Maze;

namespace visibility
{
    void compute(Maze & maze,
                 float tileScale,
                 float maxRayDistance,
                 const glm::vec3 & origin,
                 float yaw);

    bool tile_visible(const Maze & maze, int tr, int tc);
    bool world_pos_visible(const Maze & maze, float x, float z, float tileScale);
}
