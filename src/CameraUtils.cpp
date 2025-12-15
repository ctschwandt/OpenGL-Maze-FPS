#include "CameraUtils.h"

#include <algorithm>
#include <cmath>

#include "Maze.h"
#include "Player.h"

namespace camerautils
{
namespace
{
const float TOP_DOWN_ZOOM_STEP = 0.005f;
const float TOP_DOWN_ZOOM_MIN  = 0.1f;
const float TOP_DOWN_ZOOM_MAX  = 0.5f;
float top_down_zoom            = 0.2f;
}

void apply_top_down_view(const game::PlayerMovement & player_state,
                         mygllib::View & view,
                         float tile_scale,
                         const Maze & maze)
{
    float maze_span     = tile_scale * static_cast<float>(maze.tiles_n);
    float camera_height = std::max(maze_span, 120.0f) * top_down_zoom;

    view.eye(player_state.position.x, camera_height, player_state.position.z);
    view.ref(player_state.position.x, player_state.ground_height, player_state.position.z);
    view.up(0.0f, 0.0f, -1.0f);
    view.type() = mygllib::View::PERSPECTIVE;
}

void handle_top_down_zoom(const mygllib::GLFWInput & input)
{
    bool z_out = input.key_down(GLFW_KEY_MINUS);
    bool z_in  = input.key_down(GLFW_KEY_EQUAL);

    if (z_out)
    {
        top_down_zoom = std::min(TOP_DOWN_ZOOM_MAX,
                                 top_down_zoom + TOP_DOWN_ZOOM_STEP);
    }
    if (z_in)
    {
        top_down_zoom = std::max(TOP_DOWN_ZOOM_MIN,
                                 top_down_zoom - TOP_DOWN_ZOOM_STEP);
    }
}

} // namespace camerautils
