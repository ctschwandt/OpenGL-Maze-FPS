#include "mygllib/GLFWInput.h"

namespace mygllib
{
    GLFWInput::GLFWInput(GLFWwindow *window)
        : window_(window),
          last_x_(0.0),
          last_y_(0.0),
          mouse_delta_x_(0.0),
          mouse_delta_y_(0.0),
          first_mouse_(true)
    {
        // Visible, normal cursor
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        // Initialize last_x_/last_y_ to current position
        double xpos, ypos;
        glfwGetCursorPos(window_, &xpos, &ypos);
        last_x_ = xpos;
        last_y_ = ypos;
    }

    void GLFWInput::begin_frame()
    {
        double xpos, ypos;
        glfwGetCursorPos(window_, &xpos, &ypos);

        if (first_mouse_)
        {
            last_x_ = xpos;
            last_y_ = ypos;
            mouse_delta_x_ = 0.0;
            mouse_delta_y_ = 0.0;
            first_mouse_ = false;
            return;
        }

        double xoffset = xpos - last_x_;
        double yoffset = last_y_ - ypos; // invert Y so up is positive

        last_x_ = xpos;
        last_y_ = ypos;

        mouse_delta_x_ = xoffset;
        mouse_delta_y_ = yoffset;
    }

    bool GLFWInput::key_down(int key) const
    {
        int state = glfwGetKey(window_, key);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }
}
