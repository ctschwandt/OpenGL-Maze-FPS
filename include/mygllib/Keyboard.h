// File  : Keyboard.h
// Author: smaug

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <list>

namespace mygllib
{
    class GLFWInput;

    class Keyboard
    {
    public:
        static void update_from_input(const GLFWInput &input, float dt);
    };

    class KeyboardBase
    {
    };
}

#endif
