#include "Renderer.h"

#include <GL/glew.h>
#include <GL/glu.h>

#include <cmath>

#include "Globals.h"
#include "mygllib/gl3d.h"
#include "mygllib/SingletonView.h"
#include "mygllib/View.h"

namespace
{
    void draw_box(float cx, float cy, float cz, float hx, float hy, float hz)
    {
        float x0 = cx - hx, x1 = cx + hx;
        float y0 = cy - hy, y1 = cy + hy;
        float z0 = cz - hz, z1 = cz + hz;

        glBegin(GL_QUADS);

        glNormal3f(0, 0, 1);
        glVertex3f(x0, y0, z1);
        glVertex3f(x1, y0, z1);
        glVertex3f(x1, y1, z1);
        glVertex3f(x0, y1, z1);

        glNormal3f(0, 0,-1);
        glVertex3f(x1, y0, z0);
        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y1, z0);
        glVertex3f(x1, y1, z0);

        glNormal3f(-1, 0, 0);
        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y0, z1);
        glVertex3f(x0, y1, z1);
        glVertex3f(x0, y1, z0);

        glNormal3f(1, 0, 0);
        glVertex3f(x1, y0, z1);
        glVertex3f(x1, y0, z0);
        glVertex3f(x1, y1, z0);
        glVertex3f(x1, y1, z1);

        glNormal3f(0, 1, 0);
        glVertex3f(x0, y1, z1);
        glVertex3f(x1, y1, z1);
        glVertex3f(x1, y1, z0);
        glVertex3f(x0, y1, z0);

        glNormal3f(0,-1, 0);
        glVertex3f(x0, y0, z0);
        glVertex3f(x1, y0, z0);
        glVertex3f(x1, y0, z1);
        glVertex3f(x0, y0, z1);

        glEnd();
    }
}

Renderer::Renderer() = default;

void Renderer::setup_camera(const Player &player)
{
    mygllib::View & view = *(mygllib::SingletonView::getInstance());

    glm::vec3 eye = player.pos + glm::vec3(0.0f, player.height * 0.9f, 0.0f);
    glm::vec3 forward(std::cos(player.yaw), 0.0f, std::sin(player.yaw));
    glm::vec3 ref = eye + forward;

    view.eye(eye.x, eye.y, eye.z);
    view.ref(ref.x, ref.y, ref.z);
    view.up(0.0f, 1.0f, 0.0f);
    view.lookat();
}

void Renderer::draw_maze(const Maze &maze)
{
    float wallHalfHeight = 0.5f;
    int tileN = maze.tiles_n;

    glColor3f(0.2f, 0.2f, 0.2f);

    for (int tr = 0; tr < tileN; ++tr)
    {
        for (int tc = 0; tc < tileN; ++tc)
        {
            if (!maze.is_wall_tile(tr, tc))
                continue;

            float cx = tc + 0.5f;
            float cz = tr + 0.5f;
            float cy = wallHalfHeight;
            draw_box(cx, cy, cz, 0.5f, wallHalfHeight, 0.5f);
        }
    }
}

void Renderer::draw_actor(const Actor &actor, float r, float g, float b)
{
    glColor3f(r, g, b);
    draw_box(actor.pos.x, actor.pos.y + actor.height * 0.5f, actor.pos.z,
             actor.radius, actor.height * 0.5f, actor.radius);
}

void Renderer::draw_projectile(const Projectile &proj)
{
    if (!proj.alive)
        return;

    glColor3f(1.0f, 0.0f, 0.0f);
    draw_box(proj.pos.x, proj.pos.y, proj.pos.z,
             proj.radius, proj.radius, proj.radius);
}

void Renderer::render(const Maze &maze,
                     const Player &player,
                     const std::vector<Enemy> &enemies,
                     const std::vector<Projectile> &projectiles)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    setup_camera(player);

    if (globals::draw_axes)
    {
        mygllib::draw_axes();
    }

    if (globals::draw_plane)
    {
        float span = static_cast<float>(maze.tiles_n);
        mygllib::draw_xz_plane(0.0f, span, 0.0f, span);
    }

    draw_maze(maze);
    draw_actor(player, 0.1f, 0.5f, 1.0f);

    for (const Enemy &enemy : enemies)
    {
        draw_actor(enemy, 1.0f, 0.3f, 0.3f);
    }

    for (const Projectile &p : projectiles)
    {
        draw_projectile(p);
    }
}

