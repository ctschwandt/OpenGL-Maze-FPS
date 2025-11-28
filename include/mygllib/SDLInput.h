#pragma once

#include <SDL2/SDL.h>

namespace mygllib
{
    class SDLInput
    {
    public:
        SDLInput();

        void begin_frame();
        void capture_mouse_delta();

        double mouse_delta_x() const { return mouse_delta_x_; }
        double mouse_delta_y() const { return mouse_delta_y_; }

        bool key_down(SDL_Scancode key) const;

        void request_quit() { quit_requested_ = true; }
        bool quit_requested() const { return quit_requested_; }

    private:
        const Uint8 *keyboard_state_;
        double mouse_delta_x_;
        double mouse_delta_y_;
        bool quit_requested_;
    };
}

