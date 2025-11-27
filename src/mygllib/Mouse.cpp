#include <algorithm>
#include <cmath>
#include <iostream>

#include "mygllib/GLFWInput.h"
#include "mygllib/Mouse.h"
#include "mygllib/SingletonView.h"
#include "mygllib/View.h"

namespace
{
    const float  MOUSE_SENS       = 0.005f;   // tweak if needed
    const double MAX_FRAME_DELTA  = 500.0;    // safety clamp in pixels
}

void mygllib::Mouse::update_from_input(const GLFWInput &input, float /*dt*/)
{
    double dx = input.mouse_delta_x();
    double dy = input.mouse_delta_y();

    if (dx == 0.0 && dy == 0.0)
        return;

    // Safety clamp to avoid insane spins
    if (std::abs(dx) > MAX_FRAME_DELTA)
        dx = (dx > 0 ? 1 : -1) * MAX_FRAME_DELTA;
    if (std::abs(dy) > MAX_FRAME_DELTA)
        dy = (dy > 0 ? 1 : -1) * MAX_FRAME_DELTA;

    std::cout << "(dx, dy): " << dx << ' ' << dy << std::endl;

    mygllib::View & view = *(mygllib::SingletonView::getInstance());

    view.yaw()   += static_cast<float>(dx) * MOUSE_SENS;
    view.pitch() += static_cast<float>(dy) * MOUSE_SENS;

    std::cout << "(yaw, pitch): " << view.yaw() << ' ' << view.pitch() << std::endl;

    view.pitch() = std::clamp(view.pitch(), -1.2f, 1.2f);
    view.update_center_from_yaw_pitch();
}
