// File: main.cpp
// Name: Cole Schwandt

#include <exception>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "Globals.h"
#include "Maze.h"
#include "mygllib/gl3d.h"
#include "mygllib/SDLInput.h"
#include "mygllib/View.h"
#include "mygllib/SingletonView.h"
#include "mygllib/Reshape.h"
#include "mygllib/Keyboard.h"
#include "mygllib/Mouse.h"
#include "mygllib/Material.h"
#include "mygllib/Light.h"
#include "myglm.h"

//==============================================================
// Globals
//==============================================================
Maze maze(5);

//==============================================================
// Lighting
//==============================================================
//mygllib::Light light;

void init()
{
    // gl setup
    //=============================
    mygllib::View & view = *(mygllib::SingletonView::getInstance());

    view.update_center_from_yaw_pitch();

    glClearColor(1, 1, 1, 1);
    glEnable(GL_DEPTH_TEST);
    //glClearDepth(cfg::CLEAR_DEPTH);

    // === enable lighting & color material ===
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    //glFrontFace(GL_CCW);
}

//==============================================================
// Drawing Helpers
//==============================================================
// Draw an axis-aligned box given center and half-sizes
void draw_box(float cx, float cy, float cz,
              float hx, float hy, float hz)
{
    float x0 = cx - hx, x1 = cx + hx;
    float y0 = cy - hy, y1 = cy + hy;
    float z0 = cz - hz, z1 = cz + hz;

    glBegin(GL_QUADS);

    // front (z1)
    glNormal3f(0, 0, 1);
    glVertex3f(x0, y0, z1);
    glVertex3f(x1, y0, z1);
    glVertex3f(x1, y1, z1);
    glVertex3f(x0, y1, z1);

    // back (z0)
    glNormal3f(0, 0,-1);
    glVertex3f(x1, y0, z0);
    glVertex3f(x0, y0, z0);
    glVertex3f(x0, y1, z0);
    glVertex3f(x1, y1, z0);

    // left (x0)
    glNormal3f(-1, 0, 0);
    glVertex3f(x0, y0, z0);
    glVertex3f(x0, y0, z1);
    glVertex3f(x0, y1, z1);
    glVertex3f(x0, y1, z0);

    // right (x1)
    glNormal3f(1, 0, 0);
    glVertex3f(x1, y0, z1);
    glVertex3f(x1, y0, z0);
    glVertex3f(x1, y1, z0);
    glVertex3f(x1, y1, z1);

    // top (y1)
    glNormal3f(0, 1, 0);
    glVertex3f(x0, y1, z1);
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y1, z0);
    glVertex3f(x0, y1, z0);

    // bottom (y0)
    glNormal3f(0,-1, 0);
    glVertex3f(x0, y0, z0);
    glVertex3f(x1, y0, z0);
    glVertex3f(x1, y0, z1);
    glVertex3f(x0, y0, z1);

    glEnd();
}

void draw_maze_columns()
{
    float H     = 1.5f;      // wall height in logical units
    float hy    = H / 2.0f;
    int   tileN = maze.tiles_n;   // = 2*n + 1

    for (int tr = 0; tr < tileN; ++tr)
    {
        for (int tc = 0; tc < tileN; ++tc)
        {
            if (!maze.is_wall_tile(tr, tc))
                continue;

            // logical center (before any scaling)
            float cx = tc + 0.5f;
            float cz = tr + 0.5f;
            float cy = hy;          // center in Y

            // Each wall fully occupies one 1x1 tile footprint
            draw_box(cx, cy, cz,
                     0.5f, hy, 0.5f);
        }
    }
}

//==============================================================
// User Input
//==============================================================
void handle_function_keys(const mygllib::SDLInput &input)
{
    static bool f1_down_previous = false;
    static bool f2_down_previous = false;
    static bool f3_down_previous = false;

    bool f1_down = input.key_down(SDL_SCANCODE_F1);
    bool f2_down = input.key_down(SDL_SCANCODE_F2);
    bool f3_down = input.key_down(SDL_SCANCODE_F3);

    if (f1_down && !f1_down_previous)
    {
        globals::draw_plane = !globals::draw_plane;
    }
    if (f2_down && !f2_down_previous)
    {
        globals::draw_axes = !globals::draw_axes;
    }
    if (f3_down && !f3_down_previous)
    {
        globals::draw_wire = !globals::draw_wire;
    }

    f1_down_previous = f1_down;
    f2_down_previous = f2_down;
    f3_down_previous = f3_down;
}

//==============================================================
// Display
//==============================================================
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    mygllib::SingletonView::getInstance()->lookat();

    //mygllib::Light::all_off();
    glLineWidth(1.0f);

    const float S = 3.0f;
    float maze_span = S * maze.tiles_n;

    if (globals::draw_plane)
    {
        mygllib::draw_xz_plane(0.0f , maze_span, 0.0f, maze_span); //-5000.0f, 5000.0f, -5000.0f, 5000.0f);
    }
    if (globals::draw_axes)
    {
        mygllib::draw_axes(); //500.0f, 2.0f);
    }
    //mygllib::Light::all_on();
    
    //light.on();
    //glEnable(GL_NORMALIZE);
    //glShadeModel(GL_SMOOTH);
    //light.set_position();
    
    // draw maze columns
    glColor3f(0.2f, 0.2f, 0.2f);
    glPushMatrix();
    {
        glScalef(S, S, S);
        draw_maze_columns();
    }
    glPopMatrix();
    
}

//==============================================================
// main
//==============================================================
int main(int argc, char ** argv)
{
    (void)argc;
    (void)argv;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // ----- Maze generation (text debug) -----
    maze.init(0, 0);
    maze.print();
    std::cout << std::endl;

    // ----- Create window & GL context -----
    mygllib::WIN_W = 700;
    mygllib::WIN_H = 700;
    SDL_Window * window = nullptr;
    SDL_GLContext gl_context = nullptr;

    try
    {
        window = mygllib::init3d(gl_context);
    }
    catch (const std::exception & ex)
    {
        std::cerr << ex.what() << std::endl;
        return -1;
    }

    // Initial reshape
    mygllib::Reshape::reshape(mygllib::WIN_W, mygllib::WIN_H);

    // Resize callback
    init();

    // ----- Input wrapper -----
    mygllib::SDLInput input;

    // Timing for dt
    Uint64 perf_freq = SDL_GetPerformanceFrequency();
    Uint64 last_counter = SDL_GetPerformanceCounter();

    // ----- Main loop -----
    bool running = true;
    while (running)
    {
        Uint64 current_counter = SDL_GetPerformanceCounter();
        float dt = static_cast<float>(current_counter - last_counter) / static_cast<float>(perf_freq);
        last_counter = current_counter;

        input.begin_frame();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_WINDOWEVENT &&
                     event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                mygllib::Reshape::reshape(event.window.data1, event.window.data2);
            }
        }

        input.capture_mouse_delta();

        handle_function_keys(input);
        mygllib::Mouse::update_from_input(input);
        mygllib::Keyboard::update_from_input(input, dt);

        display();

        if (input.quit_requested())
            running = false;

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
