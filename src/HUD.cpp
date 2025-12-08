#include "HUD.h"

#include "Enemy.h"
#include "Player.h"
#include "mygllib/SingletonView.h"
#include "mygllib/config.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include <glm/gtx/norm.hpp>
#include <glm/vec3.hpp>

// Visibility buffer defined in main.cpp
extern std::vector<bool> g_visible_tiles;

namespace
{
    struct GlyphPattern
    {
        std::array<std::string, 5> rows{};
    };

    const GlyphPattern & glyph_for_char(char c)
    {
        static const GlyphPattern blank{{"   ", "   ", "   ", "   ", "   "}};
        static const std::array<std::pair<char, GlyphPattern>, 18> glyphs =
        {{
            { '0', {{"###", "# #", "# #", "# #", "###"}} },
            { '1', {{" ##", "  #", "  #", "  #", " ###"}} },
            { '2', {{"###", "  #", "###", "#  ", "###"}} },
            { '3', {{"###", "  #", "###", "  #", "###"}} },
            { '4', {{"# #", "# #", "###", "  #", "  #"}} },
            { '5', {{"###", "#  ", "###", "  #", "###"}} },
            { '6', {{"###", "#  ", "###", "# #", "###"}} },
            { '7', {{"###", "  #", "  #", "  #", "  #"}} },
            { '8', {{"###", "# #", "###", "# #", "###"}} },
            { '9', {{"###", "# #", "###", "  #", "###"}} },
            { 'S', {{"###", "#  ", "###", "  #", "###"}} },
            { 'C', {{"###", "#  ", "#  ", "#  ", "###"}} },
            { 'O', {{"###", "# #", "# #", "# #", "###"}} },
            { 'R', {{"###", "# #", "###", "# #", "# #"}} },
            { 'E', {{"###", "#  ", "###", "#  ", "###"}} },
            { ':', {{"   ", " # ", "   ", " # ", "   "}} },
            { 'F', {{"###", "#  ", "## ", "#  ", "#  "}} },
            { 'P', {{"###", "# #", "###", "#  ", "#  "}} }
        }};

        for (const auto & entry : glyphs)
        {
            if (entry.first == c)
                return entry.second;
        }

        return blank;
    }

    void draw_glyph(float x, float y, float cellSize, const GlyphPattern & glyph)
    {
        const float height = static_cast<float>(glyph.rows.size());

        glBegin(GL_QUADS);
        for (std::size_t row = 0; row < glyph.rows.size(); ++row)
        {
            const std::string & line = glyph.rows[row];
            for (std::size_t col = 0; col < line.size(); ++col)
            {
                if (line[col] != '#')
                    continue;

                float px = x + static_cast<float>(col) * cellSize;
                float py = y + (height - 1.0f - static_cast<float>(row)) * cellSize;

                glVertex2f(px,            py);
                glVertex2f(px + cellSize, py);
                glVertex2f(px + cellSize, py + cellSize);
                glVertex2f(px,            py + cellSize);
            }
        }
        glEnd();
    }

    void draw_block_text(float x, float y, float cellSize, const std::string & text)
    {
        float cursor = x;
        for (char ch : text)
        {
            const GlyphPattern & glyph = glyph_for_char(static_cast<char>(std::toupper(ch)));
            draw_glyph(cursor, y, cellSize, glyph);

            std::size_t width = glyph.rows.empty() ? 0 : glyph.rows.front().size();
            cursor += (static_cast<float>(width) + 1.0f) * cellSize;
        }
    }

    void draw_health_and_score(const game::PlayerMovement & playerState)
    {
        const float margin     = 20.0f;
        const float barWidth   = 200.0f;
        const float barHeight  = 20.0f;
        const float padding    = 2.0f;

        float ratio = (playerState.maxHealth > 0)
                    ? static_cast<float>(playerState.health) / static_cast<float>(playerState.maxHealth)
                    : 0.0f;
        ratio = std::clamp(ratio, 0.0f, 1.0f);

        const bool lowHealth = ratio < 0.25f;

        const float x0 = margin;
        const float y0 = margin;
        const float x1 = x0 + barWidth;
        const float y1 = y0 + barHeight;

        glColor4f(0.05f, 0.05f, 0.05f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(x0, y0);
        glVertex2f(x1, y0);
        glVertex2f(x1, y1);
        glVertex2f(x0, y1);
        glEnd();

        float fillWidth = (barWidth - 2.0f * padding) * ratio;
        if (lowHealth)
            glColor3f(0.9f, 0.15f, 0.15f);
        else
            glColor3f(0.2f, 0.8f, 0.2f);

        glBegin(GL_QUADS);
        glVertex2f(x0 + padding, y0 + padding);
        glVertex2f(x0 + padding + fillWidth, y0 + padding);
        glVertex2f(x0 + padding + fillWidth, y1 - padding);
        glVertex2f(x0 + padding, y1 - padding);
        glEnd();

        glColor3f(0.9f, 0.9f, 0.9f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x0, y0);
        glVertex2f(x1, y0);
        glVertex2f(x1, y1);
        glVertex2f(x0, y1);
        glEnd();

        glColor3f(0.95f, 0.95f, 0.95f);
        const float textCellSize = 4.0f;
        const float textYOffset  = 12.0f;
        float textY = y1 + textYOffset;
        draw_block_text(x0, textY, textCellSize, "SCORE: " + std::to_string(playerState.score));
    }

    float current_yaw()
    {
        const glm::vec3 dir = game::player_movement_state().facingDirection;
        if (glm::length2(dir) > 0.0001f)
        {
            glm::vec3 flat = glm::normalize(glm::vec3(dir.x, 0.0f, dir.z));
            return std::atan2(flat.z, flat.x);
        }

        return static_cast<float>(mygllib::SingletonView::getInstance()->yaw());
    }

    void draw_minimap(const Maze & maze, float tileScale)
    {
        const float margin   = 20.0f;
        const float mapSize  = 200.0f;
        float mapRight  = mygllib::WIN_W - margin;
        float mapTop    = mygllib::WIN_H - margin;
        float mapLeft   = mapRight - mapSize;
        float mapBottom = mapTop   - mapSize;

        // Background
        glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
        glBegin(GL_QUADS);
        glVertex2f(mapLeft - 4.0f, mapBottom - 4.0f);
        glVertex2f(mapRight + 4.0f, mapBottom - 4.0f);
        glVertex2f(mapRight + 4.0f, mapTop + 4.0f);
        glVertex2f(mapLeft - 4.0f, mapTop + 4.0f);
        glEnd();

        float tileSize = mapSize / static_cast<float>(maze.tiles_n);

        // Draw discovered tiles
        for (int tr = 0; tr < maze.tiles_n; ++tr)
        {
            for (int tc = 0; tc < maze.tiles_n; ++tc)
            {
                if (!maze.is_tile_discovered(tr, tc))
                    continue;

                bool wall = maze.is_wall_tile(tr, tc);
                bool visible = false;
                int idx = tr * maze.tiles_n + tc;
                if (idx >= 0 && idx < static_cast<int>(g_visible_tiles.size()))
                    visible = g_visible_tiles[idx];

                if (wall)
                    glColor3f(0.2f, 0.2f, 0.2f);
                else if (visible)
                    glColor3f(0.8f, 0.8f, 0.8f);
                else
                    glColor3f(0.5f, 0.5f, 0.5f);

                float x0 = mapLeft + static_cast<float>(tc) * tileSize;
                float y0 = mapTop - (static_cast<float>(tr + 1) * tileSize);
                float x1 = x0 + tileSize;
                float y1 = y0 + tileSize;

                glBegin(GL_QUADS);
                glVertex2f(x0, y0);
                glVertex2f(x1, y0);
                glVertex2f(x1, y1);
                glVertex2f(x0, y1);
                glEnd();
            }
        }

        // Player marker (triangle facing yaw)
        const game::PlayerMovement & player = game::player_movement_state();
        float px = player.position.x / tileScale;
        float pz = player.position.z / tileScale;
        float playerX = mapLeft + px * tileSize;
        float playerY = mapTop - pz * tileSize;

        float yaw = current_yaw();
        float size = std::max(3.0f, tileSize * 0.4f);

        glColor3f(0.1f, 0.9f, 0.1f);
        glBegin(GL_TRIANGLES);
        glVertex2f(playerX + std::cos(yaw) * size, playerY - std::sin(yaw) * size);
        glVertex2f(playerX + std::cos(yaw + 2.5f) * size, playerY - std::sin(yaw + 2.5f) * size);
        glVertex2f(playerX + std::cos(yaw - 2.5f) * size, playerY - std::sin(yaw - 2.5f) * size);
        glEnd();

        // Enemy blips
        glColor3f(0.9f, 0.1f, 0.1f);
        for (const auto & enemy : game::active_enemies())
        {
            float ex = enemy.pos.x / tileScale;
            float ez = enemy.pos.z / tileScale;
            float enemyX = mapLeft + ex * tileSize;
            float enemyY = mapTop - ez * tileSize;
            float half = std::max(2.0f, tileSize * 0.2f);

            glBegin(GL_QUADS);
            glVertex2f(enemyX - half, enemyY - half);
            glVertex2f(enemyX + half, enemyY - half);
            glVertex2f(enemyX + half, enemyY + half);
            glVertex2f(enemyX - half, enemyY + half);
            glEnd();
        }

        // Border
        glColor3f(0.9f, 0.9f, 0.9f);
        glLineWidth(1.5f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(mapLeft, mapBottom);
        glVertex2f(mapRight, mapBottom);
        glVertex2f(mapRight, mapTop);
        glVertex2f(mapLeft, mapTop);
        glEnd();
    }

    void draw_fps(float fps)
    {
        std::stringstream ss;
        ss << "FPS: " << static_cast<int>(std::round(fps));

        glColor3f(0.95f, 0.95f, 0.95f);
        const float cell = 4.0f;
        float x = 10.0f;
        float y = mygllib::WIN_H - 20.0f;
        draw_block_text(x, y, cell, ss.str());
    }
}

namespace game
{
    namespace
    {
        float g_smoothed_fps = 0.0f;
    }

    void update_fps(float dt)
    {
        if (dt <= 0.0f)
            return;

        float fps = 1.0f / dt;
        if (g_smoothed_fps <= 0.0f)
            g_smoothed_fps = fps;
        else
            g_smoothed_fps = 0.1f * fps + 0.9f * g_smoothed_fps;
    }

    void draw_hud(const Maze & maze, float tileScale)
    {
        glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0, mygllib::WIN_W, 0.0, mygllib::WIN_H, -1.0, 1.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        const game::PlayerMovement & playerState = game::player_movement_state();
        draw_health_and_score(playerState);
        draw_minimap(maze, tileScale);
        draw_fps(g_smoothed_fps);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

        glPopAttrib();
    }
}
