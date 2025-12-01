#include "Draw.h"

#include <GL/glew.h>
#include <cmath>

namespace game
{
    void draw_cylinder(float radius, float height, int segments)
    {
        float halfHeight = height * 0.5f;

        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, halfHeight, 0.0f);
        for (int i = 0; i <= segments; ++i)
        {
            float theta = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * static_cast<float>(M_PI);
            float x = radius * std::cos(theta);
            float z = radius * std::sin(theta);
            glVertex3f(x, halfHeight, z);
        }
        glEnd();

        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(0.0f, -halfHeight, 0.0f);
        for (int i = segments; i >= 0; --i)
        {
            float theta = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * static_cast<float>(M_PI);
            float x = radius * std::cos(theta);
            float z = radius * std::sin(theta);
            glVertex3f(x, -halfHeight, z);
        }
        glEnd();

        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= segments; ++i)
        {
            float theta = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * static_cast<float>(M_PI);
            float cosTheta = std::cos(theta);
            float sinTheta = std::sin(theta);
            float x = radius * cosTheta;
            float z = radius * sinTheta;
            glNormal3f(cosTheta, 0.0f, sinTheta);
            glVertex3f(x, -halfHeight, z);
            glVertex3f(x, halfHeight, z);
        }
        glEnd();
    }

    void draw_box(float width, float height, float depth)
    {
        const float hx = width * 0.5f;
        const float hy = height * 0.5f;
        const float hz = depth * 0.5f;

        glBegin(GL_QUADS);

        // +X face
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(hx, -hy, -hz);
        glVertex3f(hx, -hy, hz);
        glVertex3f(hx, hy, hz);
        glVertex3f(hx, hy, -hz);

        // -X face
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(-hx, -hy, -hz);
        glVertex3f(-hx, hy, -hz);
        glVertex3f(-hx, hy, hz);
        glVertex3f(-hx, -hy, hz);

        // +Y face
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-hx, hy, -hz);
        glVertex3f(hx, hy, -hz);
        glVertex3f(hx, hy, hz);
        glVertex3f(-hx, hy, hz);

        // -Y face
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(-hx, -hy, -hz);
        glVertex3f(-hx, -hy, hz);
        glVertex3f(hx, -hy, hz);
        glVertex3f(hx, -hy, -hz);

        // +Z face
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-hx, -hy, hz);
        glVertex3f(-hx, hy, hz);
        glVertex3f(hx, hy, hz);
        glVertex3f(hx, -hy, hz);

        // -Z face
        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f(-hx, -hy, -hz);
        glVertex3f(hx, -hy, -hz);
        glVertex3f(hx, hy, -hz);
        glVertex3f(-hx, hy, -hz);

        glEnd();
    }
}
