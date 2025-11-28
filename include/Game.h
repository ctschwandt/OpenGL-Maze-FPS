#pragma once

#include <vector>

#include "Maze.h"
#include "Player.h"
#include "Enemy.h"
#include "Projectile.h"
#include "InputController.h"
#include "Renderer.h"
#include "mygllib/GLFWInput.h"

class Game
{
public:
    Game();

    void update(const mygllib::GLFWInput &input, float dt);
    void render();

private:
    void spawn_test_enemy();

    Maze maze;
    Player player;
    std::vector<Enemy> enemies;
    std::vector<Projectile> projectiles;
    InputController inputController;
    Renderer renderer;
};

