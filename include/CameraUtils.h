#pragma once
#include "mygllib/View.h"
#include "mygllib/GLFWInput.h"

class Maze;
namespace game { struct PlayerMovement; }

namespace camerautils
{
    void apply_top_down_view(const game::PlayerMovement & player_state,
                             mygllib::View & view,
                             float tile_scale,
                             const Maze & maze);

    void handle_top_down_zoom(const mygllib::GLFWInput & input);
}
