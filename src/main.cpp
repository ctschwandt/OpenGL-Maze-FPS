// File: main.cpp
// Name: Cole Schwandt

#include <exception>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

#include <glm/gtx/norm.hpp>

#include "AppInit.h"
#include "CameraUtils.h"
#include "Draw.h"
#include "Enemy.h"
#include "GameFlow.h"
#include "Globals.h"
#include "HUD.h"
#include "Maze.h"
#include "MazeRender.h"
#include "Player.h"
#include "Projectile.h"
#include "Texture.h"
#include "Visibility.h"
#include "Worldbox.h"
#include "mygllib/gl3d.h"
#include "mygllib/GLFWInput.h"
#include "mygllib/Keyboard.h"
#include "mygllib/Mouse.h"
#include "mygllib/Reshape.h"
#include "mygllib/SingletonView.h"
#include "mygllib/View.h"
#include "mygllib/config.h"

//==============================================================
// User Input
//==============================================================
void handle_function_keys(const mygllib::GLFWInput & input)
{
    static bool tab_down_previous   = false;
    static bool grave_down_previous = false;
    static bool r_down_previous     = false;
    static bool m_down_previous     = false;
    static bool f1_down_previous    = false;

    bool tab_down = input.key_down(GLFW_KEY_TAB);
    bool grave_down = input.key_down(GLFW_KEY_GRAVE_ACCENT);
    bool r_down = input.key_down(GLFW_KEY_R);
    bool m_down = input.key_down(GLFW_KEY_M);
    bool f1_down = input.key_down(GLFW_KEY_F1);

    if (tab_down && !tab_down_previous)
    {
        globals::top_down_view = !globals::top_down_view;
    }

    if (grave_down && !grave_down_previous)
    {
        globals::enemy_freeze_active        = !globals::enemy_freeze_active;
        globals::enemy_freeze_used_this_run = true;
    }

    if (r_down && !r_down_previous)
    {
        gameflow::start_new_run();
    }

    if (m_down && !m_down_previous)
    {
        globals::draw_minimap = !globals::draw_minimap;
    }

    if (f1_down && !f1_down_previous)
    {
        if (globals::game_state == globals::GameState::ROBERT_CUBE)
        {
            globals::game_state = globals::GameState::MAZE;
        }
        else
        {
            globals::game_state = globals::GameState::ROBERT_CUBE;
        }
    }

    tab_down_previous   = tab_down;
    grave_down_previous = grave_down;
    r_down_previous     = r_down;
    m_down_previous     = m_down;
    f1_down_previous    = f1_down;
}

//==============================================================
// Display
//==============================================================
void display()
{
    if (globals::game_state == globals::GameState::ROBERT_CUBE)
    {
        render::draw_robert_cube();
        return;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    mygllib::SingletonView::getInstance()->lookat();

    worldbox::draw();

    glLineWidth(1.0f);

    render::draw_maze(gameflow::maze, gameflow::TILE_SCALE);

    render::draw_player(game::player_movement_state());
    if (globals::top_down_view)
        render::draw_player_indicator(game::player_movement_state());

    const auto & enemies = game::active_enemies();
    for (const auto & enemy : enemies)
    {
        if (!visibility::world_pos_visible(gameflow::maze, enemy.pos.x, enemy.pos.z, gameflow::TILE_SCALE))
            continue;

        enemy.draw();
    }

    render::draw_projectiles(gameflow::maze, gameflow::TILE_SCALE, game::active_projectiles());

    game::draw_hud(gameflow::maze, gameflow::TILE_SCALE);
}

//==============================================================
// main
//==============================================================
int main(int argc, char ** argv)
{
    (void)argc;
    (void)argv;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    std::cout << "==================== CONTROLS ====================\n";
    std::cout << "WASD          - Movement\n";
    std::cout << "Arrow Keys    - Look around\n";
    std::cout << "Enter         - Shoot\n";
    std::cout << "Space         - Jump\n";
    std::cout << "Left Shift    - Dash\n";
    std::cout << "Left Ctrl     - Slide\n";
    std::cout << "TAB           - Toggle Top-Down View\n";
    std::cout << "M             - Toggle Minimap\n";
    std::cout << "`             - Toggle Freezing Enemies\n";
    std::cout << "R             - Restart Run\n";
    std::cout << "F1            - Toggle Robert Cube Mode\n";
    std::cout << "ESC           - Quit\n";
    std::cout << "===================================================\n";

    // ----- create window -----
    mygllib::WIN_W = 1100;
    mygllib::WIN_H = 800;
    GLFWwindow * window = nullptr;

    try
    {
        window = mygllib::init3d();
    }
    catch (const std::exception & ex)
    {
        std::cerr << ex.what() << std::endl;
        return -1;
    }

    // turn on v-sync
    glfwSwapInterval(1);

    // Initial reshape
    mygllib::Reshape::reshape(mygllib::WIN_W, mygllib::WIN_H);

    // Resize callback
    glfwSetFramebufferSizeCallback(window,
        [](GLFWwindow *, int w, int h)
        {
            mygllib::Reshape::reshape(w, h);
        });

    appinit::init_gl();

    try
    {
        appinit::init_textures();
    }
    catch (const std::exception & ex)
    {
        std::cerr << ex.what() << std::endl;
        glfwTerminate();
        return -1;
    }

    gameflow::start_new_run();

    mygllib::GLFWInput input(window);

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        mygllib::View & view = *(mygllib::SingletonView::getInstance());

        input.begin_frame();
        glfwPollEvents();

        double currentTime = glfwGetTime();
        float  dt          = static_cast<float>(currentTime - lastTime);
        lastTime           = currentTime;

        worldbox::update(dt);

        // Handle input & game updates
        handle_function_keys(input);
        if (globals::game_state == globals::GameState::MAZE)
        {
            mygllib::Mouse::update_from_input(input);
        }
        mygllib::Keyboard::update_from_input(input, dt);

        if (globals::game_state == globals::GameState::MAZE)
        {
            game::update_player_movement(input, dt, view, gameflow::maze, gameflow::TILE_SCALE);
            game::update_enemies(dt, game::player_movement_state(), gameflow::maze);
            game::update_projectiles(dt, gameflow::maze, gameflow::TILE_SCALE,
                                     game::active_enemies(),
                                     game::player_movement_state());

            game::PlayerMovement & playerState = game::player_movement_state();
            if (playerState.health <= 0)
            {
                gameflow::start_new_run();
                continue;
            }

            if (gameflow::maze_had_enemies() && game::active_enemies().empty())
            {
                gameflow::start_new_run(false);
                continue;
            }

            if (globals::top_down_view)
            {
                camerautils::handle_top_down_zoom(input);
                camerautils::apply_top_down_view(playerState, view, gameflow::TILE_SCALE, gameflow::maze);
            }
            else
            {
                view.up(0.0f, 1.0f, 0.0f);
                view.update_center_from_yaw_pitch();
            }

            glm::vec3 origin = playerState.position;
            float     rayYaw = 0.0f;

            if (globals::top_down_view)
            {
                glm::vec3 dir = playerState.facing_direction;
                if (glm::length2(dir) > 0.0f)
                {
                    dir    = glm::normalize(glm::vec3(dir.x, 0.0f, dir.z));
                    rayYaw = std::atan2(dir.z, dir.x);
                }
                else
                {
                    rayYaw = static_cast<float>(view.yaw());
                }
            }
            else
            {
                rayYaw = static_cast<float>(view.yaw());
            }

            float mazeSpan = gameflow::TILE_SCALE * static_cast<float>(gameflow::maze.tiles_n);
            visibility::compute(gameflow::maze, gameflow::TILE_SCALE, mazeSpan, origin, rayYaw);
        }

        // Render
        display();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
