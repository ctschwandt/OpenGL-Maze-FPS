// File: Globals.h
// Name: Cole Schwandt

#ifndef GLOBALS_H
#define GLOBALS_H

#include <GL/glew.h>

namespace globals
{
    enum class GameState
    {
        MAZE,
        ROBERT_CUBE
    };

    extern bool draw_plane;
    extern bool draw_axes;
    extern bool draw_wire;
    extern bool draw_minimap;
    extern bool top_down_view;

    extern GLuint floor_texture;
    extern GLuint wall_texture;

    extern GLuint robert_texture;
    extern GLuint landon_texture;

    extern GameState game_state;

    extern float robert_rot_x;
    extern float robert_rot_y;

    extern bool enemy_freeze_active;
    extern bool enemy_freeze_used_this_run;
}

#endif // GLOBALS_H
