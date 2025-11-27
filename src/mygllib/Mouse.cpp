#include <algorithm>
#include <cmath>
#include <GL/freeglut.h>

#include "mygllib/Mouse.h"
#include "mygllib/SingletonView.h"
#include "mygllib/View.h"

namespace
{
    bool firstMouse    = true;
    int  lastX         = 0;
    int  lastY         = 0;
    const float MOUSE_SENS = 0.005f;
}

void mygllib::Mouse::motion(int x, int y)
{
    if (firstMouse)
    {
        lastX = x;
        lastY = y;
        firstMouse = false;
        return;
    }

    int dx = x - lastX;
    int dy = y - lastY;

    lastX = x;
    lastY = y;

    // If there's no movement, nothing to do
    if (dx == 0 && dy == 0)
        return;

    mygllib::View & view = *(mygllib::SingletonView::getInstance());

    view.yaw()   += dx * MOUSE_SENS;
    view.pitch() -= dy * MOUSE_SENS;

    view.pitch() = std::clamp(view.pitch(), -1.2f, 1.2f);

    view.update_center_from_yaw_pitch();
    glutPostRedisplay();
}

void mygllib::Mouse::button(int, int, int, int)
{
    // No mouse button handling implemented yet
}
