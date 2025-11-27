#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace mygllib
{
    class GLFWInput
    {
    public:
        explicit GLFWInput(GLFWwindow *window);

        void begin_frame();

        double mouse_delta_x() const { return mouse_delta_x_; }
        double mouse_delta_y() const { return mouse_delta_y_; }

        bool key_down(int key) const;

        GLFWwindow * window() const { return window_; }

    private:
        static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);

        GLFWwindow *window_;
        double last_x_;
        double last_y_;
        double mouse_delta_x_;
        double mouse_delta_y_;
        bool first_mouse_;
    };
}

