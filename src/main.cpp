// File: main.cpp

#include <exception>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <ctime>

#include "Globals.h"
#include "Game.h"
#include "mygllib/gl3d.h"
#include "mygllib/GLFWInput.h"
#include "mygllib/View.h"
#include "mygllib/SingletonView.h"
#include "mygllib/Reshape.h"

void init_gl()
{
    mygllib::View & view = *(mygllib::SingletonView::getInstance());
    view.update_center_from_yaw_pitch();

    glClearColor(1, 1, 1, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);
}

void handle_function_keys(const mygllib::GLFWInput &input)
{
    static bool f1_down_previous = false;
    static bool f2_down_previous = false;
    static bool f3_down_previous = false;

    bool f1_down = input.key_down(GLFW_KEY_F1);
    bool f2_down = input.key_down(GLFW_KEY_F2);
    bool f3_down = input.key_down(GLFW_KEY_F3);

    if (f1_down && !f1_down_previous)
    {
        globals::draw_plane = !globals::draw_plane;
    }
    if (f2_down && !f2_down_previous)
    {
        globals::draw_axes = !globals::draw_axes;
    }
    if (f3_down && !f3_down_previous)
    {
        globals::draw_wire = !globals::draw_wire;
    }

    f1_down_previous = f1_down;
    f2_down_previous = f2_down;
    f3_down_previous = f3_down;
}

int main(int argc, char ** argv)
{
    (void)argc;
    (void)argv;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    mygllib::WIN_W = 700;
    mygllib::WIN_H = 700;
    GLFWwindow * window = nullptr;

    try
    {
        window = mygllib::init3d();
    }
    catch (const std::exception & ex)
    {
        std::cerr << ex.what() << std::endl;
        return -1;
    }

    mygllib::Reshape::reshape(mygllib::WIN_W, mygllib::WIN_H);
    glfwSetFramebufferSizeCallback(window,
        [](GLFWwindow *, int w, int h)
        {
            mygllib::Reshape::reshape(w, h);
        });

    init_gl();

    mygllib::GLFWInput input(window);
    Game game;

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        input.begin_frame();
        glfwPollEvents();

        double currentTime = glfwGetTime();
        float dt = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        handle_function_keys(input);
        game.update(input, dt);
        game.render();

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

