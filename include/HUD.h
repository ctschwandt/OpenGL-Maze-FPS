#ifndef HUD_H
#define HUD_H

#include "Maze.h"

namespace game
{
    // Draw the on-screen HUD overlay (health, score, minimap).
    void draw_hud(const Maze & maze, float tileScale);
}

#endif // HUD_H
