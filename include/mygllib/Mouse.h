#pragma once

#include "mygllib/SDLInput.h"

namespace mygllib
{
    class Mouse
    {
    public:
        static void update_from_input(const SDLInput & input);
    };
}
