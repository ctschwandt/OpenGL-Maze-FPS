// File  : Keyboard.cpp
// Author: Cole Schwandt

#include <GL/freeglut.h>
#include "Globals.h"
#include "View.h"
#include "SingletonView.h"
#include "Keyboard.h"

void mygllib::Keyboard::keyboard(unsigned char key, int w, int h)
{
    using namespace globals;
    mygllib::View & view = *(mygllib::SingletonView::getInstance());

    const float dv = 0.1f;
    switch (key)
    {
        // camera controls
        //=======================
        case 'x': view.eyex() -= dv; break;
        case 'X': view.eyex() += dv; break;
        case 'y': view.eyey() -= dv; break;
        case 'Y': view.eyey() += dv; break;
        case 'z': view.eyez() -= dv; break;
        case 'Z': view.eyez() += dv; break;
            
        case 'v': view.fovy() -= 0.1; break;
        case 'V': view.fovy() += 0.1; break;            
        case 'a': view.aspect() -= 0.1; break;
        case 'A': view.aspect() += 0.1; break;
        case 'n': view.zNear() -= 0.1; break;
        case 'N': view.zNear() += 0.1; break;
        case 'f': view.zFar() -= 0.1; break;
        case 'F': view.zFar() += 0.1; break;

        // area control
        //=======================
        case '[':
            g_A -= 1.0f;
            if (g_A < 1.0f)
                g_A = 1.0f;
            break;

        case ']':
            g_A += 1.0f;
            break;

        // generate terrain
        //======================
        case 'g':
            g_heightmap.reset(g_n, g_M, g_r);
            break;
            
        // M control: random range
        //======================
        case 'm':              // smaller random amplitude
            g_M -= 1.0f;
            if (g_M < 0.0f) g_M = 0.0f;
            g_heightmap.reset(g_n, g_M, g_r);
            break;

        case 'M':              // larger random amplitude
            g_M += 1.0f;
            g_heightmap.reset(g_n, g_M, g_r);
            break;

        // n control: (N = 2^n + 1)
        //======================
        case 'h':              // smaller N
            if (g_n > 1)
            {
                --g_n;
                g_heightmap.reset(g_n, g_M, g_r);
            }
            break;

        case 'H':              // larger N
            if (g_n < 10)      // cap however you like
            {
                ++g_n;
                g_heightmap.reset(g_n, g_M, g_r);
            }
            break;

        // r control: roughness
        //======================
        case 'r':              // decrease roughness
            g_r -= 0.1f;
            if (g_r < 0.1f) g_r = 0.1f;
            g_heightmap.reset(g_n, g_M, g_r);
            break;

        case 'R':              // increase roughness
            g_r += 0.1f;
            if (g_r > 5.0f) g_r = 5.0f;
            g_heightmap.reset(g_n, g_M, g_r);
            break;
    }

    // clamp values
    if (view.fovy() < 15.0f)  view.fovy() = 15.f;
    if (view.fovy() > 120.0f) view.fovy() = 120.f;

    if (view.aspect() < 0.1f) view.aspect() = 0.1f;

    if (view.zNear() < 1e-4f) view.zNear() = 1e-4f;
    if (view.zFar()  < view.zNear() * 10.f) view.zFar() = view.zNear() * 10.f;

    view.set_projection();
    view.lookat();
    //light.set_position();
    glutPostRedisplay();
}
