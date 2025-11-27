// File: main.cpp
// Name: Cole Schwandt
//
// Description:
// Procedural terrain generation using diamond square algorithm

#include <iostream>
#include <GL/freeglut.h>
#include <cmath>
#include <cstdlib>
#include <vector>

#include "Globals.h"
#include "Maze.h"
#include "mygllib/gl3d.h"
#include "mygllib/View.h"
#include "mygllib/SingletonView.h"
#include "mygllib/Reshape.h"
#include "mygllib/Keyboard.h"
#include "mygllib/Material.h"
#include "mygllib/Light.h"
#include "myglm.h"

//==============================================================
// Globals
//==============================================================
Maze maze(5, 0, 0);

//==============================================================
// Lighting
//==============================================================
//mygllib::Light light;

void init()
{
    // gl setup
    //=============================
    mygllib::View & view = *(mygllib::SingletonView::getInstance());

    // GLfloat span = GLfloat(globals::g_heightmap.N_ - 1);  // size of terrain in x,z
    
    // view.eyex() = span * 0.5f;  // center in x
    // view.eyey() = span;
    // view.eyez() = span;
    // view.zFar() = span * 10.0f; // 10 times current terrain size

    view.set_projection();
    view.lookat();

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
    float H     = 3.0f;      // wall height
    float hy    = H / 2.0f;
    int   tileN = maze.tiles_n();

    for (int tr = 0; tr < tileN; ++tr)
    {
        for (int tc = 0; tc < tileN; ++tc)
        {
            if (!maze.is_wall_tile(tr, tc))
                continue;

            // Each tile is a 1x1 in XZ
            float cx = tc + 0.5f;
            float cz = tr + 0.5f;
            float cy = hy;  // center at y = 1.5

            // full 1x1 footprint in XZ, height 3
            draw_box(cx, cy, cz,
                     0.5f, hy, 0.5f);
        }
    }
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
    if (globals::draw_plane)
    {
        mygllib::draw_xz_plane(); //-5000.0f, 5000.0f, -5000.0f, 5000.0f);
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
        draw_maze_columns();
    }
    glPopMatrix();
    
    glutSwapBuffers();
}

//==============================================================
// User Input
//==============================================================
void specialkeyboard(int key, int, int)
{    
    switch (key)
    {
        case GLUT_KEY_F1: globals::draw_plane = !globals::draw_plane; break;
        case GLUT_KEY_F2: globals::draw_axes = !globals::draw_axes; break;
        case GLUT_KEY_F3: globals::draw_wire = !globals::draw_wire; break;
    }

    glutPostRedisplay();
}

//==============================================================
// main
//==============================================================
int main(int argc, char ** argv)
{
    maze.print();
    std::cout << std::endl;
    
    srand((unsigned int) time(NULL));
    mygllib::WIN_W = 700;
    mygllib::WIN_H = 700;
    mygllib::init3d();
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(mygllib::Keyboard::keyboard);
    glutSpecialFunc(specialkeyboard);
    glutReshapeFunc(mygllib::Reshape::reshape);
    glutMainLoop();
    
    return 0;
}
