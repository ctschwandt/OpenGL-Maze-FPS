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
        // visible cursor
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    #ifdef GLFW_RAW_MOUSE_MOTION
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(window_, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    #endif
    }

    void GLFWInput::begin_frame()
    {
        // reset deltas
        mouse_delta_x_ = 0.0;
        mouse_delta_y_ = 0.0;

        double xpos, ypos;
        glfwGetCursorPos(window_, &xpos, &ypos);

        if (first_mouse_)
        {
            last_x_ = xpos;
            last_y_ = ypos;
            first_mouse_ = false;
            return;
        }

        // Deltas since last frame. This is the "virtual cursor offset"
        mouse_delta_x_ = xpos - last_x_;
        mouse_delta_y_ = last_y_ - ypos;   // invert Y so moving mouse up is +dy

        last_x_ = xpos;
        last_y_ = ypos;
    }

    bool GLFWInput::key_down(int key) const
    {
        int state = glfwGetKey(window_, key);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool GLFWInput::mouse_button_down(int button) const
    {
        int state = glfwGetMouseButton(window_, button);
        return state == GLFW_PRESS;
    }
}
