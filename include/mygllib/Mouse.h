// File: Mouse.h
// Mouse input for FPS camera look

#ifndef MOUSE_H
#define MOUSE_H

namespace mygllib
{
    class Mouse
    {
    public:
        static void motion(int x, int y);
        static void button(int button, int state, int x, int y);
    };
}

#endif
