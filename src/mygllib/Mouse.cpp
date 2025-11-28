#include <algorithm>
#include <cmath>
#include <iostream>

#include "mygllib/GLFWInput.h"
#include "mygllib/Mouse.h"
#include "mygllib/SingletonView.h"
#include "mygllib/View.h"

namespace
{
    const float  MOUSE_SENS = 0.005f;
}

void mygllib::Mouse::update_from_input(const GLFWInput & input)
{
    double dx = input.mouse_delta_x();
    double dy = input.mouse_delta_y();

    if (dx == 0.0 && dy == 0.0)
        return;

    //std::cout << "(dx, dy): " << dx << ' ' << dy << std::endl;

    mygllib::View & view = *(mygllib::SingletonView::getInstance());

    view.yaw()   += static_cast<float>(dx) * MOUSE_SENS;
    view.pitch() += static_cast<float>(dy) * MOUSE_SENS;

    //std::cout << "(yaw, pitch): " << view.yaw() << ' ' << view.pitch() << std::endl;

    view.pitch() = std::clamp(view.pitch(), -1.2f, 1.2f);
    view.update_center_from_yaw_pitch();
}
