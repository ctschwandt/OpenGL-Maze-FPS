#include "Game.h"

#include <cstdlib>

#include "Globals.h"
#include "Physics.h"

Game::Game()
    : maze(5)
{
    maze.init(0, 0);
    player.pos = glm::vec3(1.0f, 0.0f, 1.0f);
    player.height = 1.8f;

    Enemy e;
    e.pos = glm::vec3(3.0f, 0.0f, 3.0f);
    enemies.push_back(e);
}

void Game::update(const mygllib::GLFWInput &input, float dt)
{
    PlayerInput pi = inputController.sample(input, dt);
    player.update(maze, pi, dt);

    for (Enemy & enemy : enemies)
    {
        enemy.update(maze, player.pos, dt);
    }

    (void)projectiles;
}

void Game::render()
{
    renderer.render(maze, player, enemies, projectiles);
}

