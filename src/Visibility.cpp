#include "Visibility.h"

#include <cmath>
#include <vector>

#include "Maze.h"

namespace visibility
{
namespace
{
std::vector<bool> g_visible_tiles;
const float PI_F = 3.14159265358979323846f;
}

void compute(Maze & maze,
             float tileScale,
             float maxRayDistance,
             const glm::vec3 & origin,
             float yaw)
{
    int tileN = maze.tiles_n;
    if (tileN <= 0)
        return;

    g_visible_tiles.assign(tileN * tileN, 0);

    auto mark_visible = [&](int tr, int tc)
    {
        if (tr < 0 || tr >= tileN || tc < 0 || tc >= tileN)
            return;

        int idx = tr * tileN + tc;
        if (idx < 0 || idx >= static_cast<int>(g_visible_tiles.size()))
            return;

        g_visible_tiles[idx] = 1;
        maze.mark_tile_discovered(tr, tc);
    };

    float eyeX = origin.x;
    float eyeZ = origin.z;

    // Always mark the player's own cell as visible so it's not hidden
    int originTc = static_cast<int>(std::floor(eyeX / tileScale));
    int originTr = static_cast<int>(std::floor(eyeZ / tileScale));
    if (originTr >= 0 && originTr < tileN && originTc >= 0 && originTc < tileN)
        mark_visible(originTr, originTc);

    const int   NUM_RAYS  = 252;
    const float FOV_DEG   = 120.0f;
    const float FOV       = FOV_DEG * (PI_F / 180.0f);
    const float HALF_FOV  = FOV * 0.5f;
    const float STEP      = tileScale * 0.25f;

    for (int i = 0; i < NUM_RAYS; ++i)
    {
        float t   = (NUM_RAYS == 1) ? 0.5f : (static_cast<float>(i) / (NUM_RAYS - 1));
        float ang = yaw - HALF_FOV + t * FOV;

        float dirX = std::cos(ang);
        float dirZ = std::sin(ang);

        float posX = eyeX;
        float posZ = eyeZ;
        float traveled = 0.0f;

        while (traveled < maxRayDistance)
        {
            posX += dirX * STEP;
            posZ += dirZ * STEP;
            traveled += STEP;

            int tc = static_cast<int>(std::floor(posX / tileScale));
            int tr = static_cast<int>(std::floor(posZ / tileScale));

            if (tr < 0 || tr >= tileN || tc < 0 || tc >= tileN)
                break;

            mark_visible(tr, tc);

            if (maze.is_wall_tile(tr, tc))
                break;
        }
    }
}

bool tile_visible(const Maze & maze, int tr, int tc)
{
    int tileN = maze.tiles_n;
    if (tr < 0 || tr >= tileN || tc < 0 || tc >= tileN)
        return false;

    int idx = tr * tileN + tc;
    if (idx < 0 || idx >= static_cast<int>(g_visible_tiles.size()))
        return false;

    return g_visible_tiles[idx] != 0;
}

bool world_pos_visible(const Maze & maze, float x, float z, float tileScale)
{
    int tc = static_cast<int>(std::floor(x / tileScale));
    int tr = static_cast<int>(std::floor(z / tileScale));
    return tile_visible(maze, tr, tc);
}

} // namespace visibility
