#pragma once

namespace worldbox
{
    void init();           // compile shaders, create DS textures
    void update(float dt); // advance time and regenerate heightmap as needed
    void draw();           // draw the inside sky sphere

    void set_palette_index(int idx);
    void set_use_diamond(bool useDiamond);
}
