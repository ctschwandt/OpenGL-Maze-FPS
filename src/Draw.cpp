#include "Draw.h"

#include <iostream>
#include <GL/glew.h>
#include <cmath>

namespace game
{
    void draw_cylinder(float radius, float height, int segments)
    {
        float half_height = height * 0.5f;

        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, half_height, 0.0f);
        for (int i = 0; i <= segments; ++i)
        {
            float theta = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * static_cast<float>(M_PI);
            float x = radius * std::cos(theta);
            float z = radius * std::sin(theta);
            glVertex3f(x, half_height, z);
        }
        glEnd();

        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(0.0f, -half_height, 0.0f);
        for (int i = segments; i >= 0; --i)
        {
            float theta = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * static_cast<float>(M_PI);
            float x = radius * std::cos(theta);
            float z = radius * std::sin(theta);
            glVertex3f(x, -half_height, z);
        }
        glEnd();

        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= segments; ++i)
        {
            float theta = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * static_cast<float>(M_PI);
            float cos_theta = std::cos(theta);
            float sin_theta = std::sin(theta);
            float x = radius * cos_theta;
            float z = radius * sin_theta;
            glNormal3f(cos_theta, 0.0f, sin_theta);
            glVertex3f(x, -half_height, z);
            glVertex3f(x, half_height, z);
        }
        glEnd();
    }

    void draw_textured_cylinder(float radius, float height, int segments)
    {
        float half_height = height * 0.5f;

        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.5f, 0.5f);
        glVertex3f(0.0f, half_height, 0.0f);
        for (int i = 0; i <= segments; ++i)
        {
            float theta = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * static_cast<float>(M_PI);
            float x = radius * std::cos(theta);
            float z = radius * std::sin(theta);
            float u = 0.5f + 0.5f * std::cos(theta);
            float v = 0.5f + 0.5f * std::sin(theta);
            glTexCoord2f(u, v);
            glVertex3f(x, half_height, z);
        }
        glEnd();

        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glTexCoord2f(0.5f, 0.5f);
        glVertex3f(0.0f, -half_height, 0.0f);
        for (int i = segments; i >= 0; --i)
        {
            float theta = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * static_cast<float>(M_PI);
            float x = radius * std::cos(theta);
            float z = radius * std::sin(theta);
            float u = 0.5f + 0.5f * std::cos(theta);
            float v = 0.5f + 0.5f * std::sin(theta);
            glTexCoord2f(u, v);
            glVertex3f(x, -half_height, z);
        }
        glEnd();

        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= segments; ++i)
        {
            float theta = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * static_cast<float>(M_PI);
            float cos_theta = std::cos(theta);
            float sin_theta = std::sin(theta);
            float x = radius * cos_theta;
            float z = radius * sin_theta;
            float u = static_cast<float>(i) / static_cast<float>(segments);
            glTexCoord2f(u, 0.0f);
            glNormal3f(cos_theta, 0.0f, sin_theta);
            glVertex3f(x, -half_height, z);
            glTexCoord2f(u, 1.0f);
            glVertex3f(x, half_height, z);
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

    void draw_sphere(float radius, int stacks, int slices)
    {
        for (int i = 0; i < stacks; ++i)
        {
            float phi1 = static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(stacks);
            float phi2 = static_cast<float>(M_PI) * static_cast<float>(i + 1) / static_cast<float>(stacks);

            float y1 = std::cos(phi1) * radius;
            float y2 = std::cos(phi2) * radius;
            float r1 = std::sin(phi1) * radius;
            float r2 = std::sin(phi2) * radius;

            glBegin(GL_TRIANGLE_STRIP);
            for (int j = 0; j <= slices; ++j)
            {
                float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(j) / static_cast<float>(slices);
                float cos_theta = std::cos(theta);
                float sin_theta = std::sin(theta);

                float x1 = r1 * cos_theta;
                float z1 = r1 * sin_theta;
                float x2 = r2 * cos_theta;
                float z2 = r2 * sin_theta;

                float s  = static_cast<float>(j) / static_cast<float>(slices);
                float t1 = static_cast<float>(i) / static_cast<float>(stacks);
                float t2 = static_cast<float>(i + 1) / static_cast<float>(stacks);

                glTexCoord2f(s, t2);
                glNormal3f(x2 / radius, y2 / radius, z2 / radius);
                glVertex3f(x2, y2, z2);

                glTexCoord2f(s, t1);
                glNormal3f(x1 / radius, y1 / radius, z1 / radius);
                glVertex3f(x1, y1, z1);
            }
            glEnd();
        }
    }
}
