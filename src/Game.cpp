#include "Game.h"

#include <GL/glew.h>

#include "Globals.h"
#include "Physics.h"

Game::Game()
    : maze(5)
{
    maze.init(0, 0);
    spawn_test_enemy();
}

void Game::spawn_test_enemy()
{
    enemies.emplace_back();
}

void Game::update(const mygllib::GLFWInput &input, float dt)
{
    PlayerInput pInput = inputController.sample(input, dt);
    player.update(maze, pInput, dt);

    for (Enemy &enemy : enemies)
    {
        enemy.update(maze, player.pos, dt);
    }

    for (Projectile &proj : projectiles)
    {
        if (!proj.alive) continue;
        proj.pos += proj.vel * dt;
    }
}

void Game::render()
{
    renderer.render(maze, player, enemies, projectiles);
}

