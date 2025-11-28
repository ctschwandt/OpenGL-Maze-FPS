// File: gl3d.h

#ifndef GL3D_H
#define GL3D_H

#include <stdexcept>

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "config.h"
#include "View.h"
#include "Material.h"

namespace mygllib
{
    using namespace mygllib;

    inline
    SDL_Window * init3d(SDL_GLContext &gl_context)
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0)
        {
            throw std::runtime_error("Failed to initialize SDL");
        }

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        SDL_Window *window = SDL_CreateWindow(
            WIN_TITLE,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            WIN_W,
            WIN_H,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        if (!window)
        {
            SDL_Quit();
            throw std::runtime_error("Failed to create SDL window");
        }

        gl_context = SDL_GL_CreateContext(window);
        if (!gl_context)
        {
            SDL_DestroyWindow(window);
            SDL_Quit();
            throw std::runtime_error("Failed to create OpenGL context");
        }

        SDL_GL_SetSwapInterval(1);

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
        {
            SDL_GL_DeleteContext(gl_context);
            SDL_DestroyWindow(window);
            SDL_Quit();
            throw std::runtime_error("Failed to initialize GLEW");
        }

        return window;
    }

    //-------------------------------------------------------------------------
    // Draw x-, y-, z- axes in red, green, blue respectively.
    // Lights should probably be turned off (if it's on) before calling this
    // function.
    //
    // USAGE:
    // void display()
    // {
    //    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //
    //    glDisable(GL_LIGHTING);
    //    mygllib::draw_axes();
    //    glEnable(GL_LIGHTING);
    //
    //    // draw objects
    // }
    //-------------------------------------------------------------------------
    inline
    void draw_axes(float length=10, float line_width=1.0)
    {
        glLineWidth(line_width);
        glBegin(GL_LINES);
        {
            glColor3f(1, 0, 0); // red
            glVertex3f(0, 0, 0);
            glVertex3f(length, 0, 0);
            glColor3f(0, 1, 0); // green
            glVertex3f(0, 0, 0);
            glVertex3f(0, length, 0);
            glColor3f(0, 0, 1); // blue
            glVertex3f(0, 0, 0);
            glVertex3f(0, 0, length);
        }
        glEnd();
    }


    inline
    void draw_xz_plane(int minx = -20, int maxx = 20,
                       int minz = -20, int maxz = 20,
                       GLfloat dx = 1.0f,
                       GLfloat dz = 1.0f)
    {
        glColor3f(0.5, 0.5, 0.5);
        glBegin(GL_LINES);
        for (float x = minx; x <= maxx; x += 1)
        {
            glVertex3f(x, -0.001f, minz);
            glVertex3f(x, -0.001f, maxz);
        }
        for (float z = minz; z <= maxz; z += 1)
        {
            glVertex3f(minx, -0.001f, z);
            glVertex3f(maxx, -0.001f, z);
        }
        glEnd();
    }
}

#endif
