#ifndef TEXT_H
#define TEXT_H

#include <string>

#include <GL/glew.h>

namespace mygllib
{
    class Text
    {
    public:
        Text(int x=0, int y=0,
             const std::string & s="",
             float scale=0.2f)
            : x_(x), y_(y), s_(s), scale_(scale)
        {}

        void draw() const
        {
            // Text rendering via FreeGLUT was removed; this is a placeholder to
            // avoid depending on GLUT. Implement font rendering here if needed.
            (void)scale_;
        }

        static void draw(int x, int y,
                         const std::string & s,
                         float scale=0.2f)
        {
            Text(x, y, s, scale).draw();
        }

    private:
        int x_, y_;
        std::string s_;
        float scale_;
    };
}
#endif // TEXT_H
