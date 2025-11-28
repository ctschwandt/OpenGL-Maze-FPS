#pragma once

#include <vector>

#include "Maze.h"
#include "Player.h"
#include "Enemy.h"
#include "Projectile.h"

class Renderer
{
public:
    Renderer();

    void render(const Maze &maze,
                const Player &player,
                const std::vector<Enemy> &enemies,
                const std::vector<Projectile> &projectiles);

private:
    void setup_camera(const Player &player);
    void draw_maze(const Maze &maze);
    void draw_actor(const Actor &actor, float r, float g, float b);
    void draw_projectile(const Projectile &proj);
};

