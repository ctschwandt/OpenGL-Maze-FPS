#include "AppInit.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <string>

#include "Globals.h"
#include "Texture.h"
#include "Worldbox.h"
#include "mygllib/SingletonView.h"
#include "mygllib/View.h"

namespace appinit
{
namespace
{
int get_palette_index(int idx)
{
    return idx - 1;
}
}

void init_gl()
{
    mygllib::View & view = *(mygllib::SingletonView::getInstance());
    view.update_center_from_yaw_pitch();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    // === enable lighting & color material ===
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_TEXTURE_2D);

    globals::robert_texture = load_texture_2d("assets/textures/robert.png");
    globals::landon_texture = load_texture_2d("assets/textures/landon.JPG");
    globals::liow_texture   = load_texture_2d("assets/textures/liow.jpg");

    // --- GLEW and worldbox shader ---
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cerr << "GLEW init failed: "
                  << reinterpret_cast<const char*>(glewGetErrorString(err))
                  << std::endl;
    }
    else
    {
        worldbox::init();
    }
}

void init_textures()
{
    static int prev_idx = 0;
    int idx = 1 + (std::rand() % 3);
    while (idx == prev_idx)
    {
        idx = 1 + (std::rand() % 3);
    }
    prev_idx = idx;

    std::string floorPath = "assets/textures/floor" + std::to_string(idx) + ".jpg";
    std::string wallPath  = "assets/textures/wall"  + std::to_string(idx) + ".jpg";

    globals::floor_texture = load_texture_2d(floorPath);
    globals::wall_texture  = load_texture_2d(wallPath);

    // Map to palette
    worldbox::set_palette_index(get_palette_index(idx));

    worldbox::set_use_diamond(idx == 2 || (idx == 1 && rand() % 2 == 0));
}

} // namespace appinit
