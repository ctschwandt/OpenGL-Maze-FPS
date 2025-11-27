#include "mygllib/GLFWInput.h"

namespace mygllib
{
    GLFWInput::GLFWInput(GLFWwindow *window)
        : window_(window),
          last_x_(0.0),
          last_y_(0.0),
          mouse_delta_x_(0.0),
          mouse_delta_y_(0.0)
    {
        glfwSetWindowUserPointer(window_, this);
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#ifdef GLFW_RAW_MOUSE_MOTION
        if (glfwRawMouseMotionSupported())
        {
            glfwSetInputMode(window_, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }
#endif
        glfwSetCursorPosCallback(window_, cursor_position_callback);

        // Initialize the cursor position so the first event does not
        // compute a huge delta from an uninitialized value.
        glfwGetCursorPos(window_, &last_x_, &last_y_);
    }

    void GLFWInput::begin_frame()
    {
        mouse_delta_x_ = 0.0;
        mouse_delta_y_ = 0.0;
    }

    bool GLFWInput::key_down(int key) const
    {
        int state = glfwGetKey(window_, key);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    void GLFWInput::cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
    {
        auto *input = reinterpret_cast<GLFWInput *>(glfwGetWindowUserPointer(window));
        if (!input)
            return;

        input->mouse_delta_x_ += xpos - input->last_x_;
        input->mouse_delta_y_ += ypos - input->last_y_;

        input->last_x_ = xpos;
        input->last_y_ = ypos;
    }
}

