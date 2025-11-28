// File  : Keyboard.h
// Author: smaug

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <list>

namespace mygllib
{
    class SDLInput;

    class Keyboard
    {
    public:
        static void update_from_input(SDLInput &input, float dt);
    };

    class KeyboardBase
    {
    };
}

#endif
