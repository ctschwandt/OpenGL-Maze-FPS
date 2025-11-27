#include <cmath>
#include "mygllib/View.h"

namespace mygllib
{
    void View::update_center_from_yaw_pitch()
    {
        float cosPitch = std::cos(pitch_);
        float sinPitch = std::sin(pitch_);
        float cosYaw   = std::cos(yaw_);
        float sinYaw   = std::sin(yaw_);

        refx_ = eyex_ + cosYaw * cosPitch;
        refy_ = eyey_ + sinPitch;
        refz_ = eyez_ + sinYaw * cosPitch;

        set_projection();
        lookat();
    }

    extern View view;
};
