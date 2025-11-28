#ifndef MOUSE_H
#define MOUSE_H

#include "mygllib/GLFWInput.h"

namespace mygllib
{
    class Mouse
    {
    public:
        static void update_from_input(const GLFWInput & input);
    };
}

#endif // MOUSE_H
