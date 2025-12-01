// File: main.cpp
// Name: Cole Schwandt

#include <algorithm>
#include <exception>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>

#include "Globals.h"
#include "Maze.h"
#include "mygllib/gl3d.h"
#include "mygllib/GLFWInput.h"
#include "mygllib/View.h"
#include "mygllib/SingletonView.h"
#include "mygllib/Reshape.h"
#include "mygllib/Keyboard.h"
#include "mygllib/Mouse.h"
#include "mygllib/Material.h"
#include "mygllib/Light.h"
#include "myglm.h"
#include "Draw.h"
#include "Enemy.h"
#include "Player.h"
#include "Projectile.h"
#include "Texture.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>

//==============================================================
// Globals
//==============================================================
Maze maze(5);
const float TILE_SCALE = 15.0f;
const float TOP_DOWN_ZOOM_STEP = 0.005f;
const float TOP_DOWN_ZOOM_MIN  = 0.1f;
const float TOP_DOWN_ZOOM_MAX  = 2.0f;
float top_down_zoom = 0.2f;
const game::EnemySpawnWeights ENEMY_SPAWN_WEIGHTS{ 1.0f, 1.0f, 1.0f, 1.0f };

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

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_TEXTURE_2D);
}

//==============================================================
// Drawing Helpers
//==============================================================
// Draw an axis-aligned textured box given center and half-sizes
void draw_textured_box(float cx, float cy, float cz,
                       float hx, float hy, float hz)
{
    float x0 = cx - hx, x1 = cx + hx;
    float y0 = cy - hy, y1 = cy + hy;
    float z0 = cz - hz, z1 = cz + hz;

    glBegin(GL_QUADS);

    // front (z1)
    glNormal3f(0, 0, 1);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y0, z1);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y0, z1);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y1, z1);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y1, z1);

    // back (z0)
    glNormal3f(0, 0,-1);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x1, y0, z0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x0, y0, z0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x0, y1, z0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x1, y1, z0);

    // left (x0)
    glNormal3f(-1, 0, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y0, z0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x0, y0, z1);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x0, y1, z1);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y1, z0);

    // right (x1)
    glNormal3f(1, 0, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x1, y0, z1);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y0, z0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y1, z0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x1, y1, z1);

    // top (y1)
    glNormal3f(0, 1, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y1, z1);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y1, z1);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y1, z0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y1, z0);

    // bottom (y0)
    glNormal3f(0,-1, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y0, z0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y0, z0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y0, z1);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y0, z1);

    glEnd();
}

void draw_maze_columns()
{
    float H     = 0.5f;      // wall height in logical units
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
            draw_textured_box(cx, cy, cz,
                              0.5f, hy, 0.5f);
        }
    }
}

void draw_maze_floor(float maze_span, int tiles_n)
{
    glBindTexture(GL_TEXTURE_2D, globals::floor_texture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    float repeat = static_cast<float>(tiles_n);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f,      0.0f);       glVertex3f(0.0f,      0.0f, 0.0f);
    glTexCoord2f(repeat,    0.0f);       glVertex3f(maze_span, 0.0f, 0.0f);
    glTexCoord2f(repeat,    repeat);     glVertex3f(maze_span, 0.0f, maze_span);
    glTexCoord2f(0.0f,      repeat);     glVertex3f(0.0f,      0.0f, maze_span);
    glEnd();
}

void init_textures()
{
    globals::floor_texture = load_texture_2d("assets/textures/floor1.jpg");
    globals::wall_texture  = load_texture_2d("assets/textures/wall1.png");
}

void draw_player_avatar(const game::PlayerMovement & playerState)
{
    const float cylinderHeight = game::PLAYER_BODY_HEIGHT;
    const float cylinderRadius = game::PLAYER_RADIUS;

    GLfloat emissive[] = {1.0f, 0.1f, 0.8f, 1.0f};
    GLfloat emissiveOff[] = {0.0f, 0.0f, 0.0f, 1.0f};

    glPushMatrix();
    {
        glTranslatef(playerState.position.x,
                     playerState.position.y + (cylinderHeight * 0.5f),
                     playerState.position.z);

        glColor3f(1.0f, 0.0f, 0.8f);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissive);
        game::draw_cylinder(cylinderRadius, cylinderHeight);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissiveOff);
    }
    glPopMatrix();
}

void draw_player_direction_indicator(const game::PlayerMovement & playerState)
{
    glm::vec3 dir(playerState.facingDirection.x, 0.0f, playerState.facingDirection.z);
    if (glm::length2(dir) == 0.0f)
        dir = glm::vec3(0.0f, 0.0f, -1.0f);

    dir = glm::normalize(dir);
    glm::vec3 perp(-dir.z, 0.0f, dir.x);
    float perpLen2 = glm::length2(perp);
    if (perpLen2 > 0.0f)
        perp /= std::sqrt(perpLen2);
    else
        perp = glm::vec3(1.0f, 0.0f, 0.0f);

    const float arrowLength   = game::PLAYER_RADIUS * 2.0f;
    const float arrowHalfW    = arrowLength * 0.35f;
    const float arrowBackDist = arrowLength * 0.35f;

    // Put the arrow slightly above the top of the player cylinder
    const float arrowHeight = playerState.position.y + game::PLAYER_BODY_HEIGHT + 0.05f;

    glm::vec3 tip        = playerState.position + dir * arrowLength;
    glm::vec3 baseCenter = playerState.position - dir * arrowBackDist;
    glm::vec3 baseLeft   = baseCenter - perp * arrowHalfW;
    glm::vec3 baseRight  = baseCenter + perp * arrowHalfW;

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_TRIANGLES);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(tip.x,       arrowHeight, tip.z);
    glVertex3f(baseLeft.x,  arrowHeight, baseLeft.z);
    glVertex3f(baseRight.x, arrowHeight, baseRight.z);
    glEnd();
    glPopAttrib();
}

void draw_projectiles(const std::vector<game::Projectile> & projectiles)
{
    const float radius = 0.2f;

    glColor3f(1.0f, 0.9f, 0.2f);
    for (const auto & p : projectiles)
    {
        draw_textured_box(p.position.x, p.position.y, p.position.z,
                          radius, radius, radius);
    }
}

//==============================================================
// Camera helpers
//==============================================================
void apply_top_down_view(const game::PlayerMovement & playerState,
                         mygllib::View & view,
                         float tileScale,
                         const Maze & maze)
{
    float mazeSpan     = tileScale * static_cast<float>(maze.tiles_n);
    float cameraHeight = std::max(mazeSpan, 120.0f) * top_down_zoom;

    view.eye(playerState.position.x, cameraHeight, playerState.position.z);
    view.ref(playerState.position.x, playerState.groundHeight, playerState.position.z);
    view.up(0.0f, 0.0f, -1.0f);
    view.type() = mygllib::View::PERSPECTIVE;
}

void handle_top_down_zoom(const mygllib::GLFWInput & input)
{
    bool z_out = input.key_down(GLFW_KEY_MINUS);
    bool z_in  = input.key_down(GLFW_KEY_EQUAL);

    if (z_out)
    {
        top_down_zoom = std::min(TOP_DOWN_ZOOM_MAX,
                                 top_down_zoom + TOP_DOWN_ZOOM_STEP);
    }
    if (z_in)
    {
        top_down_zoom = std::max(TOP_DOWN_ZOOM_MIN,
                                 top_down_zoom - TOP_DOWN_ZOOM_STEP);
    }
}

//==============================================================
// User Input
//==============================================================
void handle_function_keys(const mygllib::GLFWInput &input)
{
    static bool f1_down_previous = false;
    static bool f2_down_previous = false;
    static bool f3_down_previous = false;
    static bool f4_down_previous = false;

    bool f1_down = input.key_down(GLFW_KEY_F1);
    bool f2_down = input.key_down(GLFW_KEY_F2);
    bool f3_down = input.key_down(GLFW_KEY_F3);
    bool f4_down = input.key_down(GLFW_KEY_F4);

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
    if (f4_down && !f4_down_previous)
    {
        globals::top_down_view = !globals::top_down_view;
    }

    f1_down_previous = f1_down;
    f2_down_previous = f2_down;
    f3_down_previous = f3_down;
    f4_down_previous = f4_down;
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

    float maze_span = TILE_SCALE * maze.tiles_n;

    if (globals::draw_plane)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, globals::floor_texture);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        draw_maze_floor(maze_span, maze.tiles_n);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
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
        glScalef(TILE_SCALE, TILE_SCALE, TILE_SCALE);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, globals::wall_texture);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        draw_maze_columns();
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }
    glPopMatrix();

    draw_player_avatar(game::player_movement_state());
    if (globals::top_down_view)
        draw_player_direction_indicator(game::player_movement_state());
    game::draw_enemies(game::active_enemies());
    draw_projectiles(game::active_projectiles());

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
    int start_r = std::rand() % maze.n;
    int start_c = std::rand() % maze.n;
    glm::ivec2 playerStartCell(start_r, start_c);
    maze.init(start_r, start_c);
    maze.print();
    std::cout << std::endl;

    // ----- Create window & GL context -----
    mygllib::WIN_W = 700;
    mygllib::WIN_H = 700;
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

    // Initial reshape
    mygllib::Reshape::reshape(mygllib::WIN_W, mygllib::WIN_H);

    // Resize callback
    glfwSetFramebufferSizeCallback(window,
        [](GLFWwindow *, int w, int h)
        {
            mygllib::Reshape::reshape(w, h);
        });

    // Place the camera/player at the center of the chosen start cell
    {
        float start_x = TILE_SCALE * (2.0f * static_cast<float>(start_c) + 1.5f);
        float start_z = TILE_SCALE * (2.0f * static_cast<float>(start_r) + 1.5f);

        mygllib::View & view = *(mygllib::SingletonView::getInstance());
        float yaw = static_cast<float>(view.yaw());
        float eyeOffsetX = std::cos(yaw) * game::PLAYER_RADIUS;
        float eyeOffsetZ = std::sin(yaw) * game::PLAYER_RADIUS;
        view.eye(start_x + eyeOffsetX, game::PLAYER_EYE_HEIGHT, start_z + eyeOffsetZ);
    }

    init();

    try
    {
        init_textures();
    }
    catch (const std::exception & ex)
    {
        std::cerr << ex.what() << std::endl;
        glfwTerminate();
        return -1;
    }

    game::spawn_enemies(maze, TILE_SCALE, playerStartCell, ENEMY_SPAWN_WEIGHTS);

    // ----- Input wrapper (sets cursor disabled + callback inside ctor) -----
    mygllib::GLFWInput input(window);

    // Timing for dt
    double lastTime = glfwGetTime();

    // ----- Main loop -----
    while (!glfwWindowShouldClose(window))
    {
        mygllib::View & view = *(mygllib::SingletonView::getInstance());

        // 1) Reset deltas for this frame
        input.begin_frame();

        // 2) Pump events; this will trigger the cursor-pos callback,
        //    which accumulates mouse_delta_x_/y_ inside `input`.
        glfwPollEvents();

        // 3) Timing
        double currentTime = glfwGetTime();
        float dt = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        // 4) Handle input
        handle_function_keys(input);
        mygllib::Mouse::update_from_input(input);
        mygllib::Keyboard::update_from_input(input, dt);
        game::update_player_movement(input, dt, view, maze, TILE_SCALE);
        game::update_enemies(dt, game::player_movement_state().position, maze);
        game::update_projectiles(dt, maze, TILE_SCALE, game::active_enemies());

        if (globals::top_down_view)
        {
            handle_top_down_zoom(input);
            apply_top_down_view(game::player_movement_state(), view, TILE_SCALE, maze);
        }
        else
        {
            view.up(0.0f, 1.0f, 0.0f);
            view.update_center_from_yaw_pitch();
        }

        // 5) Render
        display();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
