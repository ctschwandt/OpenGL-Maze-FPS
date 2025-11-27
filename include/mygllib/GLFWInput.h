#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace mygllib
{
    class GLFWInput
    {
    public:
        explicit GLFWInput(GLFWwindow *window);

        // Call once per frame (AFTER glfwPollEvents)
        void begin_frame();

        // Per-frame mouse deltas (in pixels)
        double mouse_delta_x() const { return mouse_delta_x_; }
        double mouse_delta_y() const { return mouse_delta_y_; }

        bool key_down(int key) const;

        GLFWwindow * window() const { return window_; }

    private:
        GLFWwindow *window_;

        // Last cursor position
        double last_x_;
        double last_y_;

        // Per-frame deltas
        double mouse_delta_x_;
        double mouse_delta_y_;

        bool   first_mouse_;
    };
}
