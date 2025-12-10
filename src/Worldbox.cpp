#include "Worldbox.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace worldbox
{
namespace
{
//==============================================================
// Worldbox sphere shaders (GLSL 1.20, compatibility profile)
//==============================================================

const char *g_worldbox_vs = R"(#version 120
varying vec3 vDir;

void main()
{
    // Position in clip space using fixed-function matrices.
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

    // Direction from sphere center (assumed at origin in world space).
    vDir = normalize(gl_Vertex.xyz);
}
)";

const char *g_worldbox_fs = R"(#version 120
varying vec3 vDir;

uniform float uTime;
uniform int   uPaletteIndex;

// 0 = fBm / plasma, 1 = Diamond–Square heightmap
uniform int   uUseDiamond;
uniform sampler2D uDiamondTex1;
uniform sampler2D uDiamondTex2;
uniform float uBlend; // [0,1] blend between DS textures

// --- noise helpers for fBm ---

float hash(vec2 p)
{
    const vec2 k = vec2(127.1, 311.7);
    float h = dot(p, k);
    return fract(sin(h) * 43758.5453123);
}

float smoothNoise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(mix(a, b, u.x),
               mix(c, d, u.x), u.y);
}

float fbm(vec2 p)
{
    float value     = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;

    for (int i = 0; i < 6; ++i)
    {
        value     += amplitude * smoothNoise(p * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }

    return value;
}

// ---- Palettes ----
// 0: Night thunder
vec3 palette_night_thunder(float t)
{
    t = clamp(t, 0.0, 1.0);
    float x = pow(t, 1.5);

    vec3 almostBlack = vec3(0.0, 0.0, 0.02);
    vec3 darkBlue    = vec3(0.02, 0.05, 0.15);
    vec3 teal        = vec3(0.0,  0.5,  0.6);
    vec3 flash       = vec3(0.95, 0.97, 1.0);

    if (x < 0.4)  return mix(almostBlack, darkBlue, x / 0.4);
    if (x < 0.75) return mix(darkBlue,    teal,     (x - 0.4) / 0.35);
    return         mix(teal,        flash,    (x - 0.75) / 0.25);
}

// 1: Smoky ash
vec3 palette_smoke(float t)
{
    float x = clamp(t, 0.0, 1.0);
    vec3 bottom = vec3(0.0, 0.0, 0.0);
    vec3 mid    = vec3(0.12, 0.12, 0.12);
    vec3 top    = vec3(0.4, 0.4, 0.4);

    if (x < 0.5) return mix(bottom, mid, x / 0.5);
    return        mix(mid,    top, (x - 0.5) / 0.5);
}


// 2: Blue sky
vec3 palette_sky_clouds(float t)
{
    t = clamp(t, 0.0, 1.0);

    // Sky blue base gradient
    vec3 skyBottom = vec3(0.20, 0.40, 0.85);  // deeper blue
    vec3 skyTop    = vec3(0.55, 0.75, 1.00);  // lighter blue

    // Cloud layer — soft white, slightly tinted
    vec3 cloudColor = vec3(1.00, 1.00, 1.00);

    // Smoothstep gives the cloud puff softness
    float cloudMask = smoothstep(0.55, 1.0, t);

    // blend sky vertical gradient
    vec3 sky = mix(skyBottom, skyTop, t);

    // Then add clouds on top
    return mix(sky, cloudColor, cloudMask * 0.85); // 0.85 = how bright clouds get
}

vec3 palette(float t, int index)
{
    if      (index == 0) return palette_night_thunder(t);
    else if (index == 1) return palette_smoke(t);
    else return palette_sky_clouds(t);
}

//==============================================================
// Shader helpers
//==============================================================

GLuint g_worldbox_program      = 0;
GLint  g_worldbox_uTime        = -1;
GLint  g_worldbox_uPalette     = -1;
GLint  g_worldbox_uUseDiamond  = -1;
GLint  g_worldbox_uDiamondTex1 = -1;
GLint  g_worldbox_uDiamondTex2 = -1;
GLint  g_worldbox_uBlend       = -1;

float  g_world_time_sec         = 0.0f;
int    g_worldbox_palette_index = 2;   // 0=red,1=gray,2=night thunder
int    g_worldbox_useDiamond    = 0;   // 0 = fBm, 1 = Diamond-Square

// Diamond–Square textures for the worldbox
GLuint g_worldbox_heightTexA = 0;
GLuint g_worldbox_heightTexB = 0;
int    g_worldbox_hmN        = 0;
float  g_worldbox_blend      = 0.0f;
float  g_worldbox_blendSpeed = 0.5f;

// DS generation params
int   g_worldbox_n_exp = 8;    // 2^8 + 1 = 257
float g_worldbox_M     = 1.0f;
float g_worldbox_r     = 0.7f;

std::vector<float> g_worldbox_heightData;

GLuint compileShader(GLenum type, const char *src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::string log(logLen, '\0');
        glGetShaderInfoLog(shader, logLen, nullptr, log.data());
        std::cerr << "Shader compilation failed:\n" << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint createProgram(const char *vsSrc, const char *fsSrc)
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);
    if (!vs || !fs)
        return 0;

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint status = GL_FALSE;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint logLen = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLen);
        std::string log(logLen, '\0');
        glGetProgramInfoLog(prog, logLen, nullptr, log.data());
        std::cerr << "Program link failed:\n" << log << std::endl;
        glDeleteProgram(prog);
        return 0;
    }

    return prog;
}

//==============================================================
// Diamond–Square helpers
//==============================================================

float ds_random_in_range(float max_range)
{
    float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX); // [0,1]
    float centered = r * 2.0f - 1.0f; // [-1,1]
    return centered * max_range;
}

inline int ds_idx(int x, int z, int N)
{
    return z * N + x;
}

float ds_average_cardinal(const std::vector<float> & map,
                          int i, int j, int half, int N)
{
    float sum = 0.0f;
    int count = 0;

    if (j - half >= 0)
    {
        sum += map[ds_idx(i, j - half, N)];
        ++count;
    }
    if (j + half < N)
    {
        sum += map[ds_idx(i, j + half, N)];
        ++count;
    }
    if (i - half >= 0)
    {
        sum += map[ds_idx(i - half, j, N)];
        ++count;
    }
    if (i + half < N)
    {
        sum += map[ds_idx(i + half, j, N)];
        ++count;
    }

    if (count > 0)
        return sum / static_cast<float>(count);
    return 0.0f;
}

void ds_diamond_step(std::vector<float> & map, int N, int w, float Mcur)
{
    int half = w / 2;

    for (int i = half; i < N; i += w)
    {
        for (int j = half; j < N; j += w)
        {
            int idx = ds_idx(i, j, N);
            float avg = (map[ds_idx(i - half, j - half, N)] +
                         map[ds_idx(i - half, j + half, N)] +
                         map[ds_idx(i + half, j - half, N)] +
                         map[ds_idx(i + half, j + half, N)]) * 0.25f;
            map[idx] = avg + ds_random_in_range(Mcur);
        }
    }
}

void ds_square_step(std::vector<float> & map, int N, int w, float Mcur)
{
    int half = w / 2;

    for (int i = 0; i < N; i += half)
    {
        int j_start = ((i / half) % 2 == 0) ? half : 0;
        for (int j = j_start; j < N; j += w)
        {
            if (i == 0 && j == 0) continue;
            if (i == 0 && j == N - 1) continue;
            if (i == N - 1 && j == 0) continue;
            if (i == N - 1 && j == N - 1) continue;

            int idx = ds_idx(i, j, N);
            float avg = ds_average_cardinal(map, i, j, half, N);
            map[idx] = avg + ds_random_in_range(Mcur);
        }
    }
}

void ds_generate_heightmap(int n, float M, float r,
                           int &N_out, std::vector<float> &data_out)
{
    int N = (1 << n) + 1;
    N_out = N;

    data_out.assign(N * N, 0.0f);

    float Mcur = M * 0.25f;

    // corners
    data_out[ds_idx(0,     0,     N)] = ds_random_in_range(Mcur);
    data_out[ds_idx(0,     N - 1, N)] = ds_random_in_range(Mcur);
    data_out[ds_idx(N - 1, 0,     N)] = ds_random_in_range(Mcur);
    data_out[ds_idx(N - 1, N - 1, N)] = ds_random_in_range(Mcur);

    for (int w = N - 1; w >= 2; w /= 2)
    {
        ds_diamond_step(data_out, N, w, Mcur);
        ds_square_step(data_out, N, w, Mcur);
        Mcur *= std::pow(2.0f, -r);
    }
}

void ds_generate_normalized_heightmap(int n, float M, float r,
                                      int &N_out, std::vector<float> &data_out)
{
    ds_generate_heightmap(n, M, r, N_out, data_out);

    if (data_out.empty())
        return;

    float minH = data_out[0];
    float maxH = data_out[0];
    for (size_t i = 1; i < data_out.size(); ++i)
    {
        float h = data_out[i];
        if (h < minH) minH = h;
        if (h > maxH) maxH = h;
    }

    float range = maxH - minH;
    if (range > 1e-6f)
    {
        for (size_t i = 0; i < data_out.size(); ++i)
        {
            data_out[i] = (data_out[i] - minH) / range;
        }
    }
    else
    {
        for (size_t i = 0; i < data_out.size(); ++i)
            data_out[i] = 0.5f;
    }
}

void init_worldbox_heightmaps()
{
    // Generate first DS map into A
    ds_generate_normalized_heightmap(
        g_worldbox_n_exp, g_worldbox_M, g_worldbox_r,
        g_worldbox_hmN, g_worldbox_heightData);

    glGenTextures(1, &g_worldbox_heightTexA);
    glBindTexture(GL_TEXTURE_2D, g_worldbox_heightTexA);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 g_worldbox_hmN,
                 g_worldbox_hmN,
                 0,
                 GL_LUMINANCE,
                 GL_FLOAT,
                 g_worldbox_heightData.data());

    // Second DS map into B
    ds_generate_normalized_heightmap(
        g_worldbox_n_exp, g_worldbox_M, g_worldbox_r,
        g_worldbox_hmN, g_worldbox_heightData);

    glGenTextures(1, &g_worldbox_heightTexB);
    glBindTexture(GL_TEXTURE_2D, g_worldbox_heightTexB);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 g_worldbox_hmN,
                 g_worldbox_hmN,
                 0,
                 GL_LUMINANCE,
                 GL_FLOAT,
                 g_worldbox_heightData.data());

    glBindTexture(GL_TEXTURE_2D, 0);
}

// draw a big inside-view sphere around the camera
void draw_worldbox_sphere()
{
    if (!g_worldbox_program)
        return;

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT);

    // Sky should be behind everything, but we don't want it to write depth.
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE); // render both sides, it's inside

    glUseProgram(g_worldbox_program);

    if (g_worldbox_uTime >= 0)
        glUniform1f(g_worldbox_uTime, g_world_time_sec);
    if (g_worldbox_uPalette >= 0)
        glUniform1i(g_worldbox_uPalette, g_worldbox_palette_index);
    if (g_worldbox_uUseDiamond >= 0)
        glUniform1i(g_worldbox_uUseDiamond, g_worldbox_useDiamond);
    if (g_worldbox_uBlend >= 0)
        glUniform1f(g_worldbox_uBlend, g_worldbox_blend);

    // Bind DS textures to units 0 and 1 (even if not used in fBm mode)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_worldbox_heightTexA);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_worldbox_heightTexB);

    const float radius = 350.0f;   // big enough to enclose maze
    const int stacks = 32;
    const int slices = 64;

    for (int i = 0; i < stacks; ++i)
    {
        float v0   = static_cast<float>(i) / stacks;
        float v1   = static_cast<float>(i + 1) / stacks;
        float phi0 = v0 * static_cast<float>(M_PI);
        float phi1 = v1 * static_cast<float>(M_PI);

        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= slices; ++j)
        {
            float u     = static_cast<float>(j) / slices;
            float theta = u * 2.0f * static_cast<float>(M_PI);

            float x0 = std::sin(phi0) * std::cos(theta);
            float y0 = std::cos(phi0);
            float z0 = std::sin(phi0) * std::sin(theta);

            float x1 = std::sin(phi1) * std::cos(theta);
            float y1 = std::cos(phi1);
            float z1 = std::sin(phi1) * std::sin(theta);

            glVertex3f(radius * x0, radius * y0, radius * z0);
            glVertex3f(radius * x1, radius * y1, radius * z1);
        }
        glEnd();
    }

    // Unbind
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);

    glDepthMask(GL_TRUE);
    glPopAttrib();
}
} // namespace

void init()
{
    g_world_time_sec = 0.0f;

    g_worldbox_program = createProgram(g_worldbox_vs, g_worldbox_fs);
    if (!g_worldbox_program)
    {
        std::cerr << "Failed to create worldbox shader program.\n";
    }
    else
    {
        g_worldbox_uTime        = glGetUniformLocation(g_worldbox_program, "uTime");
        g_worldbox_uPalette     = glGetUniformLocation(g_worldbox_program, "uPaletteIndex");
        g_worldbox_uUseDiamond  = glGetUniformLocation(g_worldbox_program, "uUseDiamond");
        g_worldbox_uDiamondTex1 = glGetUniformLocation(g_worldbox_program, "uDiamondTex1");
        g_worldbox_uDiamondTex2 = glGetUniformLocation(g_worldbox_program, "uDiamondTex2");
        g_worldbox_uBlend       = glGetUniformLocation(g_worldbox_program, "uBlend");

        // Initialize DS textures
        init_worldbox_heightmaps();

        // Bind sampler uniforms to texture units 0 and 1
        glUseProgram(g_worldbox_program);
        if (g_worldbox_uDiamondTex1 >= 0)
            glUniform1i(g_worldbox_uDiamondTex1, 0);
        if (g_worldbox_uDiamondTex2 >= 0)
            glUniform1i(g_worldbox_uDiamondTex2, 1);
        glUseProgram(0);
    }
}

void update(float dt)
{
    g_world_time_sec += dt;

    if (g_worldbox_heightTexA && g_worldbox_heightTexB)
    {
        g_worldbox_blend += g_worldbox_blendSpeed * dt;
        if (g_worldbox_blend >= 1.0f)
        {
            g_worldbox_blend = 0.0f;

            // Swap A/B
            std::swap(g_worldbox_heightTexA, g_worldbox_heightTexB);

            // Regenerate new B
            ds_generate_normalized_heightmap(
                g_worldbox_n_exp, g_worldbox_M, g_worldbox_r,
                g_worldbox_hmN, g_worldbox_heightData);

            glBindTexture(GL_TEXTURE_2D, g_worldbox_heightTexB);
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         GL_LUMINANCE,
                         g_worldbox_hmN,
                         g_worldbox_hmN,
                         0,
                         GL_LUMINANCE,
                         GL_FLOAT,
                         g_worldbox_heightData.data());
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
}

void draw()
{
    draw_worldbox_sphere();
}

void set_palette_index(int idx)
{
    g_worldbox_palette_index = idx;
}

void set_use_diamond(bool useDiamond)
{
    g_worldbox_useDiamond = useDiamond ? 1 : 0;
}

} // namespace worldbox
