#ifndef HUD_H
#define HUD_H

#include "Maze.h"

namespace game
{
    // Update the smoothed FPS estimate using the frame delta time.
    void update_fps(float dt);

    // Draw the on-screen HUD overlay (health, score, minimap, FPS).
    void draw_hud(const Maze & maze, float tileScale);
}

#endif // HUD_H
