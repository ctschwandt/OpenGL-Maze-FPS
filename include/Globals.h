// File: Globals.h
// Name: Cole Schwandt

#ifndef GLOBALS_H
#define GLOBALS_H

#include "HeightMap.h"

namespace globals
{
    extern bool draw_plane;
    extern bool draw_axes;
    extern bool draw_wire;
    
    extern GLint g_n; // diamond-square exponent
    extern GLfloat g_M; // random range
    extern GLfloat g_r; // roughness
    extern GLfloat g_A; // area size [0,A] * [0,A]
    extern HeightMap g_heightmap;

}

#endif
