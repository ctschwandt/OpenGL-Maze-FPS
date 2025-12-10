// File: src/Texture.cpp
// Simple texture loader implemented on top of stb_image,
// written to resemble the BMP-based example from class.

#include "Texture.h"

#include <iostream>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint load_texture_2d(const std::string & path, bool flipVertically)
{
    stbi_set_flip_vertically_on_load(flipVertically ? 1 : 0);

    int width  = 0;
    int height = 0;
    int comp   = 0;

    // Request 3 components (RGB) no matter what the source is.
    // This avoids "unsupported channel count" issues.
    unsigned char * data = stbi_load(path.c_str(),
                                     &width,
                                     &height,
                                     &comp,
                                     3); // force RGB

    if (!data)
    {
        std::cerr << "Failed to load texture: " << path;
        const char * reason = stbi_failure_reason();
        if (reason)
            std::cerr << " (" << reason << ")";
        std::cerr << std::endl;
        return 0;
    }

    GLuint texid = 0;
    glGenTextures(1, &texid);

    // Select this texture
    glBindTexture(GL_TEXTURE_2D, texid);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // fixes robert cube colors

    // Wrapping parameters (repeat in S and T)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Filtering parameters (nearest-neighbor, like the example)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Upload data as RGB. requested 3 components above, so this is safe.
    glTexImage2D(GL_TEXTURE_2D,
                 0,              // level of detail (0 = base)
                 GL_RGB,         // internal format on GPU
                 width,
                 height,
                 0,              // border
                 GL_RGB,         // data format
                 GL_UNSIGNED_BYTE,
                 data);          // pointer to pixels

    // Free CPU-side image
    stbi_image_free(data);

    // Leave it bound; can unbind if wanted:
    // glBindTexture(GL_TEXTURE_2D, 0);

    return texid;
}

