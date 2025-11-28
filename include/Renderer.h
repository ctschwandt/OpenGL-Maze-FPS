#ifndef RENDERER_H
#define RENDERER_H

#pragma once

#include <vector>

#include "Maze.h"
#include "Player.h"
#include "Enemy.h"
#include "Projectile.h"

class Renderer
{
public:
    void render(const Maze &maze,
                const Player &player,
                const std::vector<Enemy> &enemies,
                const std::vector<Projectile> &projectiles);
};

#endif // RENDERER_H
