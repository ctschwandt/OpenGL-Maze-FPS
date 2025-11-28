#include "Renderer.h"

#include <GL/glew.h>
#include <cmath>

#include "Globals.h"
#include "mygllib/View.h"
#include "mygllib/SingletonView.h"
#include "mygllib/gl3d.h"

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

    void set_camera_from_player(const Player &player)
    {
        mygllib::View & view = *(mygllib::SingletonView::getInstance());
        glm::vec3 eye = player.pos + glm::vec3(0.0f, player.height * 0.9f, 0.0f);
        glm::vec3 forward = glm::vec3(std::cos(player.yaw), 0.0f, -std::sin(player.yaw));
        glm::vec3 center = eye + forward;
        view.eye(eye.x, eye.y, eye.z);
        view.ref(center.x, center.y, center.z);
        view.up(0.0f, 1.0f, 0.0f);
    }
}

void Renderer::render(const Maze &maze,
                      const Player &player,
                      const std::vector<Enemy> &enemies,
                      const std::vector<Projectile> &projectiles)
{
    (void)projectiles;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    set_camera_from_player(player);
    mygllib::SingletonView::getInstance()->lookat();

    glLineWidth(1.0f);
    const float S = 20.0f;
    float maze_span = S * maze.tiles_n;

    if (globals::draw_plane)
    {
        mygllib::draw_xz_plane(0.0f , maze_span, 0.0f, maze_span);
    }
    if (globals::draw_axes)
    {
        mygllib::draw_axes();
    }

    glColor3f(0.2f, 0.2f, 0.2f);
    glPushMatrix();
    {
        glScalef(S, S, S);
        float H = 0.5f;
        float hy = H / 2.0f;
        for (int tr = 0; tr < maze.tiles_n; ++tr)
        {
            for (int tc = 0; tc < maze.tiles_n; ++tc)
            {
                if (!maze.is_wall_tile(tr, tc))
                    continue;
                float cx = tc + 0.5f;
                float cz = tr + 0.5f;
                float cy = hy;
                draw_box(cx, cy, cz, 0.5f, hy, 0.5f);
            }
        }
    }
    glPopMatrix();

    (void)enemies;
}

