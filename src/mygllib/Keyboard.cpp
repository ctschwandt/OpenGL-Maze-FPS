// File  : Keyboard.cpp
// Author: Cole Schwandt

#include <cmath>
#include <cstdlib>
#include <GL/freeglut.h>
#include "mygllib/View.h"
#include "mygllib/SingletonView.h"
#include "mygllib/Keyboard.h"

namespace
{
    const float MOVE_SPEED = 0.2f;
    const float VERTICAL_SPEED = 0.2f;
}

void mygllib::Keyboard::keyboard(unsigned char key, int, int)
{
    mygllib::View & view = *(mygllib::SingletonView::getInstance());

    float yaw = view.yaw();
    float fx = std::cos(yaw);
    float fz = std::sin(yaw);
    float rx = -fz;
    float rz =  fx;

    bool moved = false;

    switch (key)
    {
        case 27: // ESC
            std::exit(0);
            break;
        case 'w': case 'W':
            view.eyex() += fx * MOVE_SPEED;
            view.eyez() += fz * MOVE_SPEED;
            moved = true;
            break;
        case 's': case 'S':
            view.eyex() -= fx * MOVE_SPEED;
            view.eyez() -= fz * MOVE_SPEED;
            moved = true;
            break;
        case 'a': case 'A':
            view.eyex() += rx * MOVE_SPEED;
            view.eyez() += rz * MOVE_SPEED;
            moved = true;
            break;
        case 'd': case 'D':
            view.eyex() -= rx * MOVE_SPEED;
            view.eyez() -= rz * MOVE_SPEED;
            moved = true;
            break;
        case ' ': // jump
            view.eyey() += VERTICAL_SPEED;
            moved = true;
            break;
        case 0x11: // left control key
            view.eyey() -= VERTICAL_SPEED;
            moved = true;
            break;
        default:
            break;
    }

    if (moved)
    {
        view.update_center_from_yaw_pitch();
        glutPostRedisplay();
    }
}
