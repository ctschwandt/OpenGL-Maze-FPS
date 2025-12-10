#pragma once
#include "mygllib/View.h"
#include "mygllib/GLFWInput.h"

class Maze;
namespace game { struct PlayerMovement; }

namespace camerautils
{
    void apply_top_down_view(const game::PlayerMovement & playerState,
                             mygllib::View & view,
                             float tileScale,
                             const Maze & maze);

    void handle_top_down_zoom(const mygllib::GLFWInput & input);
}
