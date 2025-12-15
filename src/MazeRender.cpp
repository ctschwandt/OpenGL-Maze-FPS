#include "MazeRender.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cmath>

#include <glm/gtx/norm.hpp>

#include "Globals.h"
#include "Draw.h"
#include "Enemy.h"
#include "Maze.h"
#include "Player.h"
#include "Projectile.h"
#include "Visibility.h"

namespace render
{
namespace
{
void draw_textured_box(float cx, float cy, float cz,
                       float hx, float hy, float hz)
{
    float x0 = cx - hx, x1 = cx + hx;
    float y0 = cy - hy, y1 = cy + hy;
    float z0 = cz - hz, z1 = cz + hz;

    glBegin(GL_QUADS);

    // front face (z+)
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y0, z1);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y0, z1);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y1, z1);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y1, z1);

    // back face (z-)
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x1, y0, z0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x0, y0, z0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x0, y1, z0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x1, y1, z0);

    // left face (x-)
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y0, z0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x0, y0, z1);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x0, y1, z1);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y1, z0);

    // right face (x+)
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x1, y0, z1);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y0, z0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y1, z0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x1, y1, z1);

    // top face (y+)
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y1, z1);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y1, z1);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y1, z0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y1, z0);

    // bottom face (y-)
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y0, z0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y0, z0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y0, z1);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y0, z1);

    glEnd();
}

void draw_maze_columns_internal(const Maze & maze)
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

            // if this cell is not visible, draw nothing contained in it
            if (!visibility::tile_visible(maze, tr, tc))
                continue;

            float cx = tc + 0.5f;
            float cz = tr + 0.5f;
            float cy = hy;          // center in Y

            draw_textured_box(cx, cy, cz,
                              0.5f, hy, 0.5f);
        }
    }
}

void draw_maze_floor_internal(const Maze & maze)
{
    int tileN = maze.tiles_n;

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);

    for (int tr = 0; tr < tileN; ++tr)
    {
        for (int tc = 0; tc < tileN; ++tc)
        {
            // Only draw floor for visible cells
            if (!visibility::tile_visible(maze, tr, tc))
                continue;

            float x0 = static_cast<float>(tc);
            float x1 = x0 + 1.0f;
            float z0 = static_cast<float>(tr);
            float z1 = z0 + 1.0f;

            float u0 = static_cast<float>(tc);
            float u1 = static_cast<float>(tc + 1);
            float v0 = static_cast<float>(tr);
            float v1 = static_cast<float>(tr + 1);

            glTexCoord2f(u0, v0); glVertex3f(x0, 0.0f, z0); // TL
            glTexCoord2f(u0, v1); glVertex3f(x0, 0.0f, z1); // BL
            glTexCoord2f(u1, v1); glVertex3f(x1, 0.0f, z1); // BR
            glTexCoord2f(u1, v0); glVertex3f(x1, 0.0f, z0); // TR
        }
    }

    glEnd();
}
}

void draw_maze(const Maze & maze, float tileScale)
{
    // ----- maze floor -----
    glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT);

    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, globals::floor_texture);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glColor3f(1.0f, 1.0f, 1.0f);

    glPushMatrix();
    glScalef(tileScale, tileScale, tileScale);
    draw_maze_floor_internal(maze);
    glPopMatrix();

    glBindTexture(GL_TEXTURE_2D, 0);
    glPopAttrib();

    // ----- maze walls -----
    glPushMatrix();
    {
        glScalef(tileScale, tileScale, tileScale);

        glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT);

        glDisable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, globals::wall_texture);

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glColor3f(1.0f, 1.0f, 1.0f);

        draw_maze_columns_internal(maze);

        glBindTexture(GL_TEXTURE_2D, 0);
        glPopAttrib();
    }
    glPopMatrix();
}

void draw_player(const game::PlayerMovement & playerState)
{
    static const float cylinderHeight = game::PLAYER_BODY_HEIGHT;
    static const float cylinderRadius = game::PLAYER_RADIUS;

    static GLfloat emissive[]    = {1.0f, 0.1f, 0.8f, 1.0f};
    static GLfloat emissiveOff[] = {0.0f, 0.0f, 0.0f, 1.0f};

    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_CULL_FACE);

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

    glPopAttrib();
}

void draw_player_indicator(const game::PlayerMovement & playerState)
{
    glm::vec3 dir(playerState.facing_direction.x, 0.0f, playerState.facing_direction.z);
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

void draw_projectiles(const Maze & maze,
                      float tileScale,
                      const std::vector<game::Projectile> & projectiles)
{
    const float radius = 0.2f;

    glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, globals::robert_texture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glColor3f(1.0f, 1.0f, 1.0f);

    for (const auto & p : projectiles)
    {
        if (!visibility::world_pos_visible(maze, p.position.x, p.position.z, tileScale))
            continue;

        // draw bullet
        draw_textured_box(p.position.x, p.position.y, p.position.z,
                          radius, radius, radius);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glPopAttrib();
}

void draw_robert_cube()
{
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use a local camera instead of maze camera
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, globals::robert_texture);
    glColor3f(1.0f, 1.0f, 1.0f);

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -5.0f);
    glRotatef(globals::robert_rot_x, 1.0f, 0.0f, 0.0f);
    glRotatef(globals::robert_rot_y, 0.0f, 1.0f, 0.0f);
    draw_textured_box(0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 2.0f);
    glPopMatrix();

    glBindTexture(GL_TEXTURE_2D, 0);
    glPopAttrib();
}

} // namespace render
