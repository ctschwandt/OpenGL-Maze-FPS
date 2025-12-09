// File: main.cpp
// Name: Cole Schwandt

#include <algorithm>
#include <exception>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <array>
#include <cctype>
#include <string>
#include <vector>
#include <cstdint>

#include "Globals.h"
#include "Maze.h"
#include "mygllib/gl3d.h"
#include "mygllib/GLFWInput.h"
#include "mygllib/View.h"
#include "mygllib/SingletonView.h"
#include "mygllib/Reshape.h"
#include "mygllib/Keyboard.h"
#include "mygllib/Mouse.h"
#include "mygllib/Material.h"
#include "mygllib/Light.h"
#include "myglm.h"
#include "Draw.h"
#include "Enemy.h"
#include "Player.h"
#include "Projectile.h"
#include "HUD.h"
#include "Texture.h"
#include "mygllib/config.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>

//==============================================================
// Worldbox sphere shaders (GLSL 1.20, compatibility profile)
//==============================================================

static const char *g_worldbox_vs = R"(#version 120
varying vec3 vDir;

void main()
{
    // Position in clip space using fixed-function matrices.
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

    // Direction from sphere center (assumed at origin in world space).
    vDir = normalize(gl_Vertex.xyz);
}
)";

static const char *g_worldbox_fs = R"(#version 120
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

void main()
{
    vec3 dir = normalize(vDir);
    float t;

    if (uUseDiamond == 0)
    {
        // --- fBm / plasma mode ---
        vec2 p = dir.xz * 4.0;
        float time = uTime * 0.25;

        float n    = fbm(p + vec2(time * 0.7,  time * 0.5));
        float warp = fbm(p * 2.5 + vec2(-time * 0.3, time * 0.2));
        t          = n + 0.5 * warp;   // ~[0,2]

        // Normalize to [0,1] with a bit of contrast
        t = (t - 0.5) * 0.8 + 0.5;
        t = clamp(t, 0.0, 1.0);
    }
    else
    {
        // --- Diamond–Square heightmap mode ---
        const float PI = 3.14159265;

        // Rotate sampling direction to move seam/poles to less noticeable locations
        mat3 rotX = mat3(
            1.0, 0.0,  0.0,
            0.0, 0.0, -1.0,
            0.0, 1.0,  0.0
        );
        vec3 dirMap = rotX * dir;

        float u = atan(dirMap.z, dirMap.x) / (2.0 * PI) + 0.5;
        float v = acos(clamp(dirMap.y, -1.0, 1.0)) / PI;
        vec2 uv = vec2(u, v);

        float h1 = texture2D(uDiamondTex1, uv).r;
        float h2 = texture2D(uDiamondTex2, uv).r;
        float h  = mix(h1, h2, clamp(uBlend, 0.0, 1.0));

        t = clamp(h, 0.0, 1.0);
    }

    vec3 color = palette(t, uPaletteIndex);
    gl_FragColor = vec4(color, 1.0);
}
)";

//==============================================================
// Shader helpers for worldbox
//==============================================================

GLuint g_worldbox_program      = 0;
GLint  g_worldbox_uTime        = -1;
GLint  g_worldbox_uPalette     = -1;
GLint  g_worldbox_uUseDiamond  = -1;
GLint  g_worldbox_uDiamondTex1 = -1;
GLint  g_worldbox_uDiamondTex2 = -1;
GLint  g_worldbox_uBlend       = -1;

float  g_world_time_sec        = 0.0f;
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

static std::vector<float> g_worldbox_heightData;

static GLuint compileShader(GLenum type, const char *src)
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

static GLuint createProgram(const char *vsSrc, const char *fsSrc)
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
// Diamond–Square heightmap for worldbox
//==============================================================

static float ds_random_in_range(float max_range)
{
    float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX); // [0,1]
    float centered = r * 2.0f - 1.0f; // [-1,1]
    return centered * max_range;
}

static inline int ds_idx(int x, int z, int N)
{
    return z * N + x;
}

static float ds_average_cardinal(const std::vector<float> &map,
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

    return (count > 0) ? (sum / static_cast<float>(count)) : 0.0f;
}

static void ds_diamond_step(std::vector<float> &map, int N, int w, float M)
{
    int half = w / 2;

    for (int x = 0; x < N - 1; x += w)
    {
        for (int z = 0; z < N - 1; z += w)
        {
            int cx = x + half;
            int cz = z + half;

            float NW = map[ds_idx(x,     z,     N)];
            float NE = map[ds_idx(x + w, z,     N)];
            float SW = map[ds_idx(x,     z + w, N)];
            float SE = map[ds_idx(x + w, z + w, N)];

            float avg = (NW + NE + SW + SE) / 4.0f;
            map[ds_idx(cx, cz, N)] = avg + ds_random_in_range(M);
        }
    }
}

static void ds_square_step(std::vector<float> &map, int N, int w, float M)
{
    int half = w / 2;

    for (int x = 0; x < N; x += half)
    {
        int start_z = ((x / half) % 2 == 0) ? half : 0;

        for (int z = start_z; z < N; z += w)
        {
            float avg = ds_average_cardinal(map, x, z, half, N);
            map[ds_idx(x, z, N)] = avg + ds_random_in_range(M);
        }
    }
}

static void ds_generate_heightmap(int n, float M, float r,
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

static void ds_generate_normalized_heightmap(int n, float M, float r,
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

static void init_worldbox_heightmaps()
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
static void draw_worldbox_sphere()
{
    if (!g_worldbox_program)
        return;

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT);

    // Sky should be behind everything, but we don't want it to write depth.
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE); // render both sides, we're inside

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

    const float radius = 300.0f;   // big enough to enclose maze
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

//==============================================================
// Game globals
//==============================================================
Maze maze(5);
const float TILE_SCALE = 15.0f;
const float TOP_DOWN_ZOOM_STEP = 0.005f;
const float TOP_DOWN_ZOOM_MIN = 0.1f;
const float TOP_DOWN_ZOOM_MAX = 0.5f;
float top_down_zoom = 0.2f;
const game::EnemySpawnWeights ENEMY_SPAWN_WEIGHTS{ 1.0f, 1.0f, 1.0f, 1.0f };

bool maze_had_enemies = false;

// Visibility mask: 1 = visible, 0 = not visible
std::vector<bool> g_visible_tiles;

const float PI_F = 3.14159265358979323846f;

//==============================================================
// Helpers
//==============================================================
glm::ivec2 random_start_cell()
{
    int start_r = std::rand() % maze.n;
    int start_c = std::rand() % maze.n;
    return glm::ivec2(start_r, start_c);
}

int get_palette_index(int idx)
{
    return idx - 1;
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
    g_worldbox_palette_index = get_palette_index(idx);

    g_worldbox_useDiamond = (idx == 2 || (idx == 1 && rand() % 2 == 0));
}

void place_player_at_cell(const glm::ivec2 & cell)
{
    float start_x = TILE_SCALE * (2.0f * static_cast<float>(cell.y) + 1.5f);
    float start_z = TILE_SCALE * (2.0f * static_cast<float>(cell.x) + 1.5f);

    mygllib::View & view = *(mygllib::SingletonView::getInstance());
    float yaw = static_cast<float>(view.yaw());
    float eyeOffsetX = std::cos(yaw) * game::PLAYER_RADIUS;
    float eyeOffsetZ = std::sin(yaw) * game::PLAYER_RADIUS;
    view.eye(start_x + eyeOffsetX, game::PLAYER_EYE_HEIGHT, start_z + eyeOffsetZ);
    view.update_center_from_yaw_pitch();
}

void reset_player_state_for_spawn(bool resetStats)
{
    game::PlayerMovement & playerState = game::player_movement_state();
    const int preservedHealth    = playerState.health;
    const int preservedMaxHealth = playerState.maxHealth;
    const int preservedScore     = playerState.score;

    playerState = game::PlayerMovement();

    if (!resetStats)
    {
        playerState.health    = preservedHealth;
        playerState.maxHealth = preservedMaxHealth;
        playerState.score     = preservedScore;
    }
    playerState.initialized = false;
}

void start_new_run(bool resetPlayerStats = true)
{
    glm::ivec2 playerStartCell = random_start_cell();

    globals::enemy_freeze_active = false;
    globals::enemy_freeze_used_this_run = false;

    maze.init(playerStartCell.x, playerStartCell.y);
    //maze.print();
    //std::cout << std::endl;

    place_player_at_cell(playerStartCell);
    reset_player_state_for_spawn(resetPlayerStats);
    game::active_projectiles().clear();
    game::spawn_enemies(maze, TILE_SCALE, playerStartCell, ENEMY_SPAWN_WEIGHTS);
    maze_had_enemies = !game::active_enemies().empty();
    init_textures();
}

//==============================================================
// Visibility helpers (cell-based)
//==============================================================
inline bool tile_visible(int tr, int tc)
{
    int tileN = maze.tiles_n;
    if (tr < 0 || tr >= tileN || tc < 0 || tc >= tileN)
        return false;

    int idx = tr * tileN + tc;
    if (idx < 0 || idx >= static_cast<int>(g_visible_tiles.size()))
        return false;

    return g_visible_tiles[idx] != 0;
}

inline bool world_pos_visible(float x, float z)
{
    int tc = static_cast<int>(std::floor(x / TILE_SCALE));
    int tr = static_cast<int>(std::floor(z / TILE_SCALE));
    return tile_visible(tr, tc);
}

//==============================================================
// Raycasting / Visibility
//==============================================================
void compute_visibility_mask(Maze & maze,
                             float tileScale,
                             float maxRayDistance,
                             const glm::vec3 & origin,
                             float yaw)
{
    int tileN = maze.tiles_n;
    if (tileN <= 0)
        return;

    g_visible_tiles.assign(tileN * tileN, 0);

    auto mark_visible = [&](int tr, int tc)
    {
        if (tr < 0 || tr >= tileN || tc < 0 || tc >= tileN)
            return;

        int idx = tr * tileN + tc;
        if (idx < 0 || idx >= static_cast<int>(g_visible_tiles.size()))
            return;

        g_visible_tiles[idx] = 1;
        maze.mark_tile_discovered(tr, tc);
    };

    float eyeX = origin.x;
    float eyeZ = origin.z;

    // Always mark the player's own cell as visible so it's not hidden
    int originTc = static_cast<int>(std::floor(eyeX / tileScale));
    int originTr = static_cast<int>(std::floor(eyeZ / tileScale));
    if (originTr >= 0 && originTr < tileN && originTc >= 0 && originTc < tileN)
        mark_visible(originTr, originTc);

    const int   NUM_RAYS = 720;
    const float FOV_DEG  = 120.0f; // 100 degrees
    const float FOV = FOV_DEG * (PI_F / 180.0f);
    const float HALF_FOV = FOV * 0.5f;
    const float STEP = tileScale * 0.25f;

    for (int i = 0; i < NUM_RAYS; ++i)
    {
        float t   = (NUM_RAYS == 1) ? 0.5f : (static_cast<float>(i) / (NUM_RAYS - 1));
        float ang = yaw - HALF_FOV + t * FOV;

        float dirX = std::cos(ang);
        float dirZ = std::sin(ang);

        float posX = eyeX;
        float posZ = eyeZ;
        float traveled = 0.0f;

        while (traveled < maxRayDistance)
        {
            posX += dirX * STEP;
            posZ += dirZ * STEP;
            traveled += STEP;

            int tc = static_cast<int>(std::floor(posX / tileScale));
            int tr = static_cast<int>(std::floor(posZ / tileScale));

            if (tr < 0 || tr >= tileN || tc < 0 || tc >= tileN)
                break;

            mark_visible(tr, tc);

            if (maze.is_wall_tile(tr, tc))
                break;
        }
    }
}

//==============================================================
// Lighting / GL init
//==============================================================
mygllib::Light light;

void init()
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
}

//==============================================================
// Drawing Helpers
//==============================================================
void draw_textured_box(float cx, float cy, float cz,
                       float hx, float hy, float hz)
{
    float x0 = cx - hx, x1 = cx + hx;
    float y0 = cy - hy, y1 = cy + hy;
    float z0 = cz - hz, z1 = cz + hz;

    glBegin(GL_QUADS);

    // front (z1)
    glNormal3f(0, 0, 1);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y0, z1);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y0, z1);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y1, z1);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y1, z1);

    // back (z0)
    glNormal3f(0, 0,-1);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x1, y0, z0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x0, y0, z0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x0, y1, z0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x1, y1, z0);

    // left (x0)
    glNormal3f(-1, 0, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y0, z0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x0, y0, z1);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x0, y1, z1);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y1, z0);

    // right (x1)
    glNormal3f(1, 0, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x1, y0, z1);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y0, z0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y1, z0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x1, y1, z1);

    // top (y1)
    glNormal3f(0, 1, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y1, z1);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y1, z1);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y1, z0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y1, z0);

    // bottom (y0)
    glNormal3f(0,-1, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y0, z0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y0, z0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y0, z1);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y0, z1);

    glEnd();
}

void draw_maze_columns()
{
    float H     = 0.5f;      // wall height in logical units
    float hy    = H / 2.0f;
    int   tileN = maze.tiles_n;   // = 2*n + 1

    for (int tr = 0; tr < tileN; ++tr)
    {
        for (int tc = 0; tc < tileN; ++tc)
        {
            if (!maze.is_wall_tile(tr, tc))
                continue;

            // if this cell is not visible, draw nothing contained in it
            if (!tile_visible(tr, tc))
                continue;

            float cx = tc + 0.5f;
            float cz = tr + 0.5f;
            float cy = hy;          // center in Y

            draw_textured_box(cx, cy, cz,
                              0.5f, hy, 0.5f);
        }
    }
}

void draw_maze_floor()
{
    int tileN = maze.tiles_n;

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);

    for (int tr = 0; tr < tileN; ++tr)
    {
        for (int tc = 0; tc < tileN; ++tc)
        {
            // Only draw floor for visible cells
            if (!tile_visible(tr, tc))
                continue;

            float x0 = static_cast<float>(tc);
            float x1 = x0 + 1.0f;
            float z0 = static_cast<float>(tr);
            float z1 = z0 + 1.0f;

            float u0 = static_cast<float>(tc);
            float u1 = static_cast<float>(tc + 1);
            float v0 = static_cast<float>(tr);
            float v1 = static_cast<float>(tr + 1);

            glTexCoord2f(u0, v0); glVertex3f(x0, 0.0f, z0); // TL
            glTexCoord2f(u0, v1); glVertex3f(x0, 0.0f, z1); // BL
            glTexCoord2f(u1, v1); glVertex3f(x1, 0.0f, z1); // BR
            glTexCoord2f(u1, v0); glVertex3f(x1, 0.0f, z0); // TR
        }
    }

    glEnd();
}

void draw_player_avatar(const game::PlayerMovement & playerState)
{
    static const float cylinderHeight = game::PLAYER_BODY_HEIGHT;
    static const float cylinderRadius = game::PLAYER_RADIUS;

    static GLfloat emissive[]    = {1.0f, 0.1f, 0.8f, 1.0f};
    static GLfloat emissiveOff[] = {0.0f, 0.0f, 0.0f, 1.0f};

    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_CULL_FACE);

    glPushMatrix();
    {
        glTranslatef(playerState.position.x,
                     playerState.position.y + (cylinderHeight * 0.5f),
                     playerState.position.z);

        glColor3f(1.0f, 0.0f, 0.8f);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissive);
        game::draw_cylinder(cylinderRadius, cylinderHeight);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissiveOff);
    }
    glPopMatrix();

    glPopAttrib();
}

void draw_player_direction_indicator(const game::PlayerMovement & playerState)
{
    glm::vec3 dir(playerState.facingDirection.x, 0.0f, playerState.facingDirection.z);
    if (glm::length2(dir) == 0.0f)
        dir = glm::vec3(0.0f, 0.0f, -1.0f);

    dir = glm::normalize(dir);
    glm::vec3 perp(-dir.z, 0.0f, dir.x);
    float perpLen2 = glm::length2(perp);
    if (perpLen2 > 0.0f)
        perp /= std::sqrt(perpLen2);
    else
        perp = glm::vec3(1.0f, 0.0f, 0.0f);

    const float arrowLength = game::PLAYER_RADIUS * 2.0f;
    const float arrowHalfW = arrowLength * 0.35f;
    const float arrowBackDist = arrowLength * 0.35f;

    const float arrowHeight = playerState.position.y + game::PLAYER_BODY_HEIGHT + 0.05f;

    glm::vec3 tip = playerState.position + dir * arrowLength;
    glm::vec3 baseCenter = playerState.position - dir * arrowBackDist;
    glm::vec3 baseLeft = baseCenter - perp * arrowHalfW;
    glm::vec3 baseRight = baseCenter + perp * arrowHalfW;

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_TRIANGLES);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(tip.x,       arrowHeight, tip.z);
    glVertex3f(baseLeft.x,  arrowHeight, baseLeft.z);
    glVertex3f(baseRight.x, arrowHeight, baseRight.z);
    glEnd();
    glPopAttrib();
}

void draw_projectiles(const std::vector<game::Projectile> & projectiles)
{
    const float radius = 0.2f;

    glColor3f(1.0f, 0.9f, 0.2f);
    for (const auto & p : projectiles)
    {
        if (!world_pos_visible(p.position.x, p.position.z))
            continue;

        draw_textured_box(p.position.x, p.position.y, p.position.z,
                          radius, radius, radius);
    }
}

//==============================================================
// Camera helpers
//==============================================================
void apply_top_down_view(const game::PlayerMovement & playerState,
                         mygllib::View & view,
                         float tileScale,
                         const Maze & maze)
{
    float mazeSpan = tileScale * static_cast<float>(maze.tiles_n);
    float cameraHeight = std::max(mazeSpan, 120.0f) * top_down_zoom;

    view.eye(playerState.position.x, cameraHeight, playerState.position.z);
    view.ref(playerState.position.x, playerState.groundHeight, playerState.position.z);
    view.up(0.0f, 0.0f, -1.0f);
    view.type() = mygllib::View::PERSPECTIVE;
}

void handle_top_down_zoom(const mygllib::GLFWInput & input)
{
    bool z_out = input.key_down(GLFW_KEY_MINUS);
    bool z_in  = input.key_down(GLFW_KEY_EQUAL);

    if (z_out)
    {
        top_down_zoom = std::min(TOP_DOWN_ZOOM_MAX,
                                 top_down_zoom + TOP_DOWN_ZOOM_STEP);
    }
    if (z_in)
    {
        top_down_zoom = std::max(TOP_DOWN_ZOOM_MIN,
                                 top_down_zoom - TOP_DOWN_ZOOM_STEP);
    }
}

//==============================================================
// User Input
//==============================================================
void handle_function_keys(const mygllib::GLFWInput & input)
{
    static bool tab_down_previous = false;
    static bool grave_down_previous = false;
    static bool r_down_previous = false;
    static bool m_down_previous = false;
    static bool f1_down_previous = false;

    bool tab_down = input.key_down(GLFW_KEY_TAB);
    bool grave_down = input.key_down(GLFW_KEY_GRAVE_ACCENT);
    bool r_down = input.key_down(GLFW_KEY_R);
    bool m_down = input.key_down(GLFW_KEY_M);
    bool f1_down = input.key_down(GLFW_KEY_F1);

    if (tab_down && !tab_down_previous)
    {
        globals::top_down_view = !globals::top_down_view;
    }

    if (grave_down && !grave_down_previous)
    {
        globals::enemy_freeze_active        = !globals::enemy_freeze_active;
        globals::enemy_freeze_used_this_run = true;
    }

    if (r_down && !r_down_previous)
    {
        start_new_run();
    }

    if (m_down && !m_down_previous)
    {
        globals::draw_minimap = !globals::draw_minimap;
    }

    if (f1_down && !f1_down_previous)
    {
        if (globals::game_state == globals::GameState::ROBERT_CUBE)
        {
            globals::game_state = globals::GameState::MAZE;
        }
        else
        {
            globals::game_state = globals::GameState::ROBERT_CUBE;
        }
    }

    tab_down_previous = tab_down;
    grave_down_previous = grave_down;
    r_down_previous = r_down;
    m_down_previous = m_down;
    f1_down_previous = f1_down;
}

//==============================================================
// Display
//==============================================================
void display()
{
    // ================== ROBERT CUBE MODE ==================
    if (globals::game_state == globals::GameState::ROBERT_CUBE)
    {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // use a local camera instead of maze camera
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, globals::robert_texture);
        glColor3f(1.0f, 1.0f, 1.0f);

        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -5.0f);
        glRotatef(globals::robert_rot_x, 1.0f, 0.0f, 0.0f);
        glRotatef(globals::robert_rot_y, 0.0f, 1.0f, 0.0f);
        draw_textured_box(0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 2.0f);
        glPopMatrix();

        glBindTexture(GL_TEXTURE_2D, 0);
        glPopAttrib();
        return;
    }

    // ================== MAZE MODE ==================//    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    mygllib::SingletonView::getInstance()->lookat();

    // ----- worldbox sphere -----
    draw_worldbox_sphere();

    glLineWidth(1.0f);

    // ----- maze floor -----
    glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT);

    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, globals::floor_texture);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glColor3f(1.0f, 1.0f, 1.0f);

    glPushMatrix();
    glScalef(TILE_SCALE, TILE_SCALE, TILE_SCALE);
    draw_maze_floor();
    glPopMatrix();

    glBindTexture(GL_TEXTURE_2D, 0);
    glPopAttrib();

    // ----- maze walls -----
    glPushMatrix();
    {
        glScalef(TILE_SCALE, TILE_SCALE, TILE_SCALE);

        glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT);

        glDisable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, globals::wall_texture);

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glColor3f(1.0f, 1.0f, 1.0f);

        draw_maze_columns();

        glBindTexture(GL_TEXTURE_2D, 0);
        glPopAttrib();
    }
    glPopMatrix();

    // ----- player, enemies, projectiles -----
    draw_player_avatar(game::player_movement_state());
    if (globals::top_down_view)
        draw_player_direction_indicator(game::player_movement_state());

    const auto & enemies = game::active_enemies();
    for (const auto & enemy : enemies)
    {
        if (!world_pos_visible(enemy.pos.x, enemy.pos.z))
            continue;

        enemy.draw();
    }

    draw_projectiles(game::active_projectiles());

    game::draw_hud(maze, TILE_SCALE);
}

//==============================================================
// main
//==============================================================
int main(int argc, char ** argv)
{
    (void)argc;
    (void)argv;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    std::cout << "==================== CONTROLS ====================\n";
    std::cout << "WASD          - Movement\n";
    std::cout << "Arrow Keys    - Look around\n";
    std::cout << "Enter         - Shoot\n";
    std::cout << "Space         - Jump\n";
    std::cout << "Left Shift    - Dash\n";
    std::cout << "Left Ctrl     - Slide\n";
    std::cout << "TAB           - Toggle Top-Down View\n";
    std::cout << "M             - Toggle Minimap\n";
    std::cout << "`             - Toggle Freezing Enemies\n";
    std::cout << "R             - Restart Run\n";
    std::cout << "F1            - Toggle Robert Cube Mode\n";
    std::cout << "ESC           - Quit\n";
    std::cout << "===================================================\n";

    // ----- create window -----
    mygllib::WIN_W = 1100;
    mygllib::WIN_H = 800;
    GLFWwindow * window = nullptr;

    try
    {
        window = mygllib::init3d();
    }
    catch (const std::exception & ex)
    {
        std::cerr << ex.what() << std::endl;
        return -1;
    }

    // turn on v-sync
    glfwSwapInterval(1);

    // Initial reshape
    mygllib::Reshape::reshape(mygllib::WIN_W, mygllib::WIN_H);

    // Resize callback
    glfwSetFramebufferSizeCallback(window,
        [](GLFWwindow *, int w, int h)
        {
            mygllib::Reshape::reshape(w, h);
        });

    init();

    try
    {
        init_textures();
    }
    catch (const std::exception & ex)
    {
        std::cerr << ex.what() << std::endl;
        glfwTerminate();
        return -1;
    }

    start_new_run();

    mygllib::GLFWInput input(window);

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        mygllib::View & view = *(mygllib::SingletonView::getInstance());

        input.begin_frame();
        glfwPollEvents();

        double currentTime = glfwGetTime();
        float dt = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        // advance worldbox animation time
        g_world_time_sec += dt;

        // Animate DS morphing regardless of mode
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

        // 4) Handle input & game updates
        handle_function_keys(input);
        if (globals::game_state == globals::GameState::MAZE)
        {
            mygllib::Mouse::update_from_input(input);
        }
        mygllib::Keyboard::update_from_input(input, dt);

        if (globals::game_state == globals::GameState::MAZE)
        {
            game::update_player_movement(input, dt, view, maze, TILE_SCALE);
            game::update_enemies(dt, game::player_movement_state(), maze);
            game::update_projectiles(dt, maze, TILE_SCALE,
                                     game::active_enemies(),
                                     game::player_movement_state());

            game::PlayerMovement & playerState = game::player_movement_state();
            if (playerState.health <= 0)
            {
                start_new_run();
                continue;
            }

            if (maze_had_enemies && game::active_enemies().empty())
            {
                start_new_run(false);
                continue;
            }

            if (globals::top_down_view)
            {
                handle_top_down_zoom(input);
                apply_top_down_view(playerState, view, TILE_SCALE, maze);
            }
            else
            {
                view.up(0.0f, 1.0f, 0.0f);
                view.update_center_from_yaw_pitch();
            }

            glm::vec3 origin = playerState.position;
            float rayYaw = 0.0f;

            if (globals::top_down_view)
            {
                glm::vec3 dir = playerState.facingDirection;
                if (glm::length2(dir) > 0.0f)
                {
                    dir = glm::normalize(glm::vec3(dir.x, 0.0f, dir.z));
                    rayYaw = std::atan2(dir.z, dir.x);
                }
                else
                {
                    rayYaw = static_cast<float>(view.yaw());
                }
            }
            else
            {
                rayYaw = static_cast<float>(view.yaw());
            }

            float mazeSpan = TILE_SCALE * static_cast<float>(maze.tiles_n);
            compute_visibility_mask(maze, TILE_SCALE, mazeSpan, origin, rayYaw);
        }

        // 5) Render
        display();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
