// File: Globals.cpp
// Name: Cole Schwandt

#include "Globals.h"

namespace globals
{
    bool draw_plane = true;
    bool draw_axes = true;
    bool draw_wire = false;

    GLint g_n = 6;
    GLfloat g_M = 60.0f;
    GLfloat g_r = 0.5f;
    GLfloat g_A = 50.0f;
    HeightMap g_heightmap(g_n, g_M, g_r);
}
