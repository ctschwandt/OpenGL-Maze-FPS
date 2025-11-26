// File: main.cpp
// Name: Cole Schwandt
//
// Description:
// Procedural terrain generation using diamond square algorithm

#include <iostream>
#include <GL/freeglut.h>
#include <cmath>
#include <cstdlib>
#include <vector>

#include "Globals.h"
#include "gl3d.h"
#include "View.h"
#include "SingletonView.h"
#include "Reshape.h"
#include "Keyboard.h"
#include "Material.h"
#include "Light.h"
#include "myglm.h"

//==============================================================
// Lighting
//==============================================================
//mygllib::Light light;

void init()
{
    // gl setup
    //=============================
    mygllib::View & view = *(mygllib::SingletonView::getInstance());

    GLfloat span = GLfloat(globals::g_heightmap.N_ - 1);  // size of terrain in x,z
    
    view.eyex() = span * 0.5f;  // center in x
    view.eyey() = span;
    view.eyez() = span;
    view.zFar() = span * 10.0f; // 10 times current terrain size

    view.set_projection();
    view.lookat();

    glClearColor(1, 1, 1, 1);
    glEnable(GL_DEPTH_TEST);
    //glClearDepth(cfg::CLEAR_DEPTH);

    // === enable lighting & color material ===
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);   // because we scale the model

    glFrontFace(GL_CCW);
}

//==============================================================
// Simple 3D vector for normals etc.
//==============================================================
struct Vec3f
{
    GLfloat x, y, z;

    Vec3f(GLfloat X = 0.0f, GLfloat Y = 0.0f, GLfloat Z = 0.0f)
        : x(X), y(Y), z(Z) {}

    Vec3f operator+(const Vec3f & o) const
    {
        return Vec3f(x + o.x, y + o.y, z + o.z);
    }

    Vec3f & operator+=(const Vec3f & o)
    {
        x += o.x;
        y += o.y;
        z += o.z;
        return *this;
    }

    Vec3f operator-(const Vec3f & o) const
    {
        return Vec3f(x - o.x, y - o.y, z - o.z);
    }

    Vec3f operator*(GLfloat s) const
    {
        return Vec3f(x * s, y * s, z * s);
    }
};

// cross product u × v
inline Vec3f cross(const Vec3f & u, const Vec3f & v)
{
    return Vec3f(
        u.y * v.z - u.z * v.y,
        u.z * v.x - u.x * v.z,
        u.x * v.y - u.y * v.x
    );
}

inline Vec3f normalize(const Vec3f & v)
{
    GLfloat len2 = v.x * v.x + v.y * v.y + v.z * v.z;
    if (len2 <= 1e-8f)
    {
        // default up if degenerate
        return Vec3f(0.0f, 1.0f, 0.0f);
    }
    GLfloat inv = 1.0f / std::sqrt(len2);
    return Vec3f(v.x * inv, v.y * inv, v.z * inv);
}

//==============================================================
// Build smooth per-vertex normals from the heightmap.
// normals[x][z] = averaged normal at grid vertex (x,z)
//==============================================================
void compute_vertex_normals(const HeightMap & hm,
                            std::vector<std::vector<Vec3f>> & normals)
{
    const GLint N = hm.N_;

    // allocate and zero
    normals.assign(N, std::vector<Vec3f>(N, Vec3f()));

    for (GLint z = 0; z < N - 1; ++z)
    {
        for (GLint x = 0; x < N - 1; ++x)
        {
            // positions in grid space (before scaling/translation)
            Vec3f p00(GLfloat(x),     hm.map_[x    ][z    ], GLfloat(z));
            Vec3f p01(GLfloat(x),     hm.map_[x    ][z + 1], GLfloat(z + 1));
            Vec3f p10(GLfloat(x + 1), hm.map_[x + 1][z    ], GLfloat(z));
            Vec3f p11(GLfloat(x + 1), hm.map_[x + 1][z + 1], GLfloat(z + 1));

            // Triangle 1: (x,z) -> (x,z+1) -> (x+1,z) (CCW from above)
            {
                Vec3f u = p01 - p00;
                Vec3f v = p10 - p00;
                Vec3f n = normalize(cross(u, v));

                normals[x    ][z    ] += n; // p00
                normals[x    ][z + 1] += n; // p01
                normals[x + 1][z    ] += n; // p10
            }

            // Triangle 2: (x+1,z) -> (x,z+1) -> (x+1,z+1)   (CCW from above)
            {
                Vec3f u = p01 - p10;
                Vec3f v = p11 - p10;
                Vec3f n = normalize(cross(u, v));

                normals[x + 1][z    ] += n; // p10
                normals[x    ][z + 1] += n; // p01
                normals[x + 1][z + 1] += n; // p11
            }
        }
    }

    // normalize all vertex normals
    for (GLint z = 0; z < N; ++z)
    {
        for (GLint x = 0; x < N; ++x)
        {
            normals[x][z] = normalize(normals[x][z]);
        }
    }
}

//==============================================================
// Glut drawing helpers
//==============================================================
void build_heightmap_mesh(const HeightMap & hm,
                          std::vector<GLfloat> & vertices,
                          std::vector<GLfloat> & normals,
                          std::vector<GLfloat> & colors,
                          std::vector<GLuint>  & indices)
{
    const GLint N = hm.N_;
    const GLint numVerts = N * N;

    // --- positions and height range (for color) ---
    vertices.resize(numVerts * 3);

    GLfloat minH = hm.map_[0][0];
    GLfloat maxH = hm.map_[0][0];

    for (GLint z = 0; z < N; ++z)
    {
        for (GLint x = 0; x < N; ++x)
        {
            GLint idx = z * N + x;
            GLfloat vx = GLfloat(x);
            GLfloat vy = hm.map_[x][z];
            GLfloat vz = GLfloat(z);

            vertices[3*idx + 0] = vx;
            vertices[3*idx + 1] = vy;
            vertices[3*idx + 2] = vz;

            if (vy < minH) minH = vy;
            if (vy > maxH) maxH = vy;
        }
    }

    GLfloat rangeH = (maxH > minH) ? (maxH - minH) : 1.0f;

    // --- vertex normals via your Vec3f helper ---
    std::vector<std::vector<Vec3f>> vnormals;
    compute_vertex_normals(hm, vnormals);

    normals.resize(numVerts * 3);
    colors.resize(numVerts * 3);

    for (GLint z = 0; z < N; ++z)
    {
        for (GLint x = 0; x < N; ++x)
        {
            GLint idx = z * N + x;

            // normal
            const Vec3f & n = vnormals[x][z];
            normals[3*idx + 0] = n.x;
            normals[3*idx + 1] = n.y;
            normals[3*idx + 2] = n.z;

            // height-based color (near white scaled by height)
            GLfloat vy = hm.map_[x][z];
            GLfloat t = (vy - minH) / rangeH;   // in [0, 1]
            GLfloat base = 0.9f * t;
            colors[3*idx + 0] = base;
            colors[3*idx + 1] = base;
            colors[3*idx + 2] = base;
        }
    }

    // --- triangle indices ---
    indices.clear();
    indices.reserve((N - 1) * (N - 1) * 6); // 2 triangles * 3 verts each

    for (GLint z = 0; z < N - 1; ++z)
    {
        for (GLint x = 0; x < N - 1; ++x)
        {
            GLuint i00 = z     * N + x;
            GLuint i01 = (z+1) * N + x;
            GLuint i10 = z     * N + (x+1);
            GLuint i11 = (z+1) * N + (x+1);

            // Triangle 1: (x,z) -> (x,z+1) -> (x+1,z)  (CCW from above)
            indices.push_back(i00);
            indices.push_back(i01);
            indices.push_back(i10);

            // Triangle 2: (x+1,z) -> (x,z+1) -> (x+1,z+1)  (CCW)
            indices.push_back(i10);
            indices.push_back(i01);
            indices.push_back(i11);
        }
    }
}

void draw_heightmap_wireframe(const HeightMap & hm)
{
    static std::vector<GLfloat> vertices;
    static std::vector<GLfloat> dummy_normals;
    static std::vector<GLfloat> dummy_colors;
    static std::vector<GLuint>  indices;

    // Rebuild mesh data (positions + indices). Normals/colors are ignored here.
    build_heightmap_mesh(hm, vertices, dummy_normals, dummy_colors, indices);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // wireframe

    // For wireframe, simple flat color is fine
    glDisable(GL_LIGHTING);
    glColor3f(0.0f, 0.0f, 0.0f);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices.data());

    glDrawElements(GL_TRIANGLES,
                   static_cast<GLsizei>(indices.size()),
                   GL_UNSIGNED_INT,
                   indices.data());

    glDisableClientState(GL_VERTEX_ARRAY);

    glEnable(GL_LIGHTING);                       // restore if you use lighting elsewhere
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // restore fill mode
}

//==============================================================
// Draw the terrain as a lit surface using vertex arrays + glDrawElements
//==============================================================
void draw_heightmap_surface(const HeightMap & hm)
{
    static std::vector<GLfloat> vertices;
    static std::vector<GLfloat> normals;
    static std::vector<GLfloat> colors;
    static std::vector<GLuint>  indices;

    build_heightmap_mesh(hm, vertices, normals, colors, indices);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glFrontFace(GL_CCW);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertices.data());
    glNormalPointer(GL_FLOAT, 0, normals.data());
    glColorPointer(3, GL_FLOAT, 0, colors.data());

    glDrawElements(GL_TRIANGLES,
                   static_cast<GLsizei>(indices.size()),
                   GL_UNSIGNED_INT,
                   indices.data());

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

//==============================================================
// Display
//==============================================================
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    mygllib::SingletonView::getInstance()->lookat();

    //mygllib::Light::all_off();
    glLineWidth(1.0f);
    if (globals::draw_plane)
    {
        mygllib::draw_xz_plane(); //-5000.0f, 5000.0f, -5000.0f, 5000.0f);
    }
    if (globals::draw_axes)
    {
        mygllib::draw_axes(); //500.0f, 2.0f);
    }
    //mygllib::Light::all_on();
    
    //light.on();
    //glEnable(GL_NORMALIZE);
    //glShadeModel(GL_SMOOTH);
    //light.set_position();
    
    glColor3f(0.0f, 0.0f, 0.0f);
    glPushMatrix();
    {
        GLfloat s = globals::g_A / GLfloat(globals::g_heightmap.N_ - 1);
        GLfloat half = 0.5f * (globals::g_heightmap.N_ - 1);
        glScalef(s, 1.0f, s);
        glTranslatef(-half, 0.0f, -half);
        if (globals::draw_wire)
            draw_heightmap_wireframe(globals::g_heightmap);
        else
            draw_heightmap_surface(globals::g_heightmap);
    }
    glPopMatrix();
    
    glutSwapBuffers();
}

//==============================================================
// User Input
//==============================================================
void specialkeyboard(int key, int, int)
{    
    switch (key)
    {
        case GLUT_KEY_F1: globals::draw_plane = !globals::draw_plane; break;
        case GLUT_KEY_F2: globals::draw_axes = !globals::draw_axes; break;
        case GLUT_KEY_F3: globals::draw_wire = !globals::draw_wire; break;
    }

    glutPostRedisplay();
}

//==============================================================
// main
//==============================================================
int main(int argc, char ** argv)
{   
    srand((unsigned int) time(NULL));
    mygllib::WIN_W = 700;
    mygllib::WIN_H = 700;
    mygllib::init3d();
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(mygllib::Keyboard::keyboard);
    glutSpecialFunc(specialkeyboard);
    glutReshapeFunc(mygllib::Reshape::reshape);
    glutMainLoop();
    
    return 0;
}
