#include "mygllib/SDLInput.h"

namespace mygllib
{
    SDLInput::SDLInput()
        : keyboard_state_(SDL_GetKeyboardState(nullptr)),
          mouse_delta_x_(0.0),
          mouse_delta_y_(0.0),
          quit_requested_(false)
    {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }

    void SDLInput::begin_frame()
    {
        mouse_delta_x_ = 0.0;
        mouse_delta_y_ = 0.0;
    }

    void SDLInput::capture_mouse_delta()
    {
        int dx = 0;
        int dy = 0;
        SDL_GetRelativeMouseState(&dx, &dy);

        mouse_delta_x_ = static_cast<double>(dx);
        mouse_delta_y_ = static_cast<double>(-dy);
    }

    bool SDLInput::key_down(SDL_Scancode key) const
    {
        return keyboard_state_[key] != 0;
    }
}

