// File: Mouse.h
// Mouse input for FPS camera look

#ifndef MOUSE_H
#define MOUSE_H

namespace mygllib
{
    class GLFWInput;

    class Mouse
    {
    public:
        static void update_from_input(const GLFWInput & input);
    };
}

#endif
