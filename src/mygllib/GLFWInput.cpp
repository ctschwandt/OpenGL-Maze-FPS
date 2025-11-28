#include "mygllib/GLFWInput.h"

namespace
{
    // Cursor position state for computing deltas
    static double lastX = 0.0;
    static double lastY = 0.0;
    static bool   firstMouse = true;

    // Accumulated deltas between frames
    static double accumulatedDeltaX = 0.0;
    static double accumulatedDeltaY = 0.0;

    void mouse_callback(GLFWwindow *window, double xpos, double ypos)
    {
        (void)window;

        // 1) On the very first event, just initialize and return
        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
            return;
        }

        // 2) Compute delta from previous position
        double xoffset = xpos - lastX;
        double yoffset = ypos - lastY;
        lastX = xpos;
        lastY = ypos;

        // 3) Invert Y so moving mouse up is +dy
        accumulatedDeltaX += xoffset;
        accumulatedDeltaY += -yoffset;
    }
}

namespace mygllib
{
    GLFWInput::GLFWInput(GLFWwindow *window)
        : window_(window),
          mouse_delta_x_(0.0),
          mouse_delta_y_(0.0)
    {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        firstMouse = true;
        glfwSetCursorPosCallback(window_, mouse_callback);

    #ifdef GLFW_RAW_MOUSE_MOTION
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(window_, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    #endif
    }

    void GLFWInput::begin_frame()
    {
        // Use the per-frame deltas gathered by the cursor callback
        mouse_delta_x_ = accumulatedDeltaX;
        mouse_delta_y_ = accumulatedDeltaY;

        accumulatedDeltaX = 0.0;
        accumulatedDeltaY = 0.0;
    }

    bool GLFWInput::key_down(int key) const
    {
        int state = glfwGetKey(window_, key);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }
}
