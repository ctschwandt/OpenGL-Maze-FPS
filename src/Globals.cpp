// File: Globals.cpp
// Name: Cole Schwandt

#include "Globals.h"

namespace globals
{
    bool draw_plane = false;
    bool draw_axes = false;
    bool draw_wire = false;
    bool draw_minimap = true;
    bool top_down_view = false;

    GLuint floor_texture = 0;
    GLuint wall_texture = 0;

    GLuint robert_texture = 0;
    GLuint landon_texture = 0;

    GameState game_state = GameState::MAZE;

    float robert_rot_x = 0.0f;
    float robert_rot_y = 0.0f;

    bool enemy_freeze_active = false;
    bool enemy_freeze_used_this_run = false;
}
