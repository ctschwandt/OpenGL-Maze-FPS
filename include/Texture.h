#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include <string>

// Load a 2D texture from disk using stb_image.
// Throws std::runtime_error on failure.
GLuint load_texture_2d(const std::string & path, bool flipVertically = true);

#endif // TEXTURE_H
