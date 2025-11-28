#pragma once

#include <vector>

#include "Maze.h"
#include "Player.h"
#include "Enemy.h"
#include "Projectile.h"
#include "InputController.h"
#include "Renderer.h"

class Game
{
public:
    Game();

    void update(const mygllib::GLFWInput &input, float dt);
    void render();

    Maze maze;
    Player player;
    std::vector<Enemy> enemies;
    std::vector<Projectile> projectiles;

private:
    InputController inputController;
    Renderer renderer;
};

