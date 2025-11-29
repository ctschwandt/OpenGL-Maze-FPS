#include "Enemy.h"

#include <algorithm>
#include <cmath>
#include <GL/glew.h>
#include <glm/gtx/norm.hpp>

#include "Maze.h"
#include "Player.h"

namespace
{
    constexpr float BULLET_SPEED          = 35.0f;
    constexpr float PLAYER_BULLET_DAMAGE  = 30.0f;
    constexpr float ENEMY_BULLET_DAMAGE   = 20.0f;
    constexpr float CHARGER_MELEE_DAMAGE  = 40.0f;
    constexpr float CHARGER_SPEED         = 45.0f;
    constexpr float CHARGER_DURATION      = 0.9f;
    constexpr float CYLINDER_SPEED        = 10.0f;
    constexpr float DRONE_SPEED           = 6.0f;

    glm::vec3 clamp_to_maze(const glm::vec3 & pos, float tileScale, const Maze & maze)
    {
        float minX = tileScale;
        float minZ = tileScale;
        float maxX = tileScale * static_cast<float>(maze.tiles_n - 1);
        float maxZ = tileScale * static_cast<float>(maze.tiles_n - 1);

        glm::vec3 clamped = pos;
        clamped.x = std::clamp(clamped.x, minX, maxX);
        clamped.z = std::clamp(clamped.z, minZ, maxZ);
        return clamped;
    }

    bool collides_with_wall(const Maze & maze, float worldX, float worldZ, float radius, float tileScale)
    {
        int x0 = static_cast<int>(std::floor((worldX - radius) / tileScale));
        int x1 = static_cast<int>(std::floor((worldX + radius) / tileScale));
        int z0 = static_cast<int>(std::floor((worldZ - radius) / tileScale));
        int z1 = static_cast<int>(std::floor((worldZ + radius) / tileScale));

        for (int tr = z0; tr <= z1; ++tr)
        {
            for (int tc = x0; tc <= x1; ++tc)
            {
                if (maze.is_wall_tile(tr, tc))
                    return true;
            }
        }

        return false;
    }

    glm::vec3 cell_center_world(int r, int c, float tileScale)
    {
        return glm::vec3(
            tileScale * (2.0f * static_cast<float>(c) + 1.5f),
            0.0f,
            tileScale * (2.0f * static_cast<float>(r) + 1.5f));
    }

    void draw_cube(float size)
    {
        float h = size * 0.5f;
        glBegin(GL_QUADS);
        glNormal3f(0, 0, 1);
        glVertex3f(-h, -h, h); glVertex3f(h, -h, h); glVertex3f(h, h, h); glVertex3f(-h, h, h);
        glNormal3f(0, 0, -1);
        glVertex3f(h, -h, -h); glVertex3f(-h, -h, -h); glVertex3f(-h, h, -h); glVertex3f(h, h, -h);
        glNormal3f(-1, 0, 0);
        glVertex3f(-h, -h, -h); glVertex3f(-h, -h, h); glVertex3f(-h, h, h); glVertex3f(-h, h, -h);
        glNormal3f(1, 0, 0);
        glVertex3f(h, -h, h); glVertex3f(h, -h, -h); glVertex3f(h, h, -h); glVertex3f(h, h, h);
        glNormal3f(0, 1, 0);
        glVertex3f(-h, h, h); glVertex3f(h, h, h); glVertex3f(h, h, -h); glVertex3f(-h, h, -h);
        glNormal3f(0, -1, 0);
        glVertex3f(-h, -h, -h); glVertex3f(h, -h, -h); glVertex3f(h, -h, h); glVertex3f(-h, -h, h);
        glEnd();
    }

    void draw_sphere(float radius, int rings = 14, int sectors = 18)
    {
        for (int r = 0; r < rings; ++r)
        {
            float theta1 = static_cast<float>(r) / static_cast<float>(rings) * static_cast<float>(M_PI);
            float theta2 = static_cast<float>(r + 1) / static_cast<float>(rings) * static_cast<float>(M_PI);

            glBegin(GL_TRIANGLE_STRIP);
            for (int s = 0; s <= sectors; ++s)
            {
                float phi = static_cast<float>(s) / static_cast<float>(sectors) * 2.0f * static_cast<float>(M_PI);

                float x1 = std::cos(phi) * std::sin(theta1);
                float y1 = std::cos(theta1);
                float z1 = std::sin(phi) * std::sin(theta1);

                float x2 = std::cos(phi) * std::sin(theta2);
                float y2 = std::cos(theta2);
                float z2 = std::sin(phi) * std::sin(theta2);

                glNormal3f(x2, y2, z2);
                glVertex3f(radius * x2, radius * y2, radius * z2);

                glNormal3f(x1, y1, z1);
                glVertex3f(radius * x1, radius * y1, radius * z1);
            }
            glEnd();
        }
    }

    void draw_prism(float length, float thickness)
    {
        glPushMatrix();
        glScalef(thickness, thickness * 0.5f, length);
        draw_cube(1.0f);
        glPopMatrix();
    }

    void draw_cylinder(float radius, float height, int segments = 18)
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
}

namespace game
{
    void spawn_default_enemies(std::vector<Enemy> & enemies,
                               const Maze & maze,
                               float tileScale,
                               const glm::vec3 & playerPos)
    {
        enemies.clear();

        for (int r = 0; r < maze.n; ++r)
        {
            for (int c = 0; c < maze.n; ++c)
            {
                glm::vec3 center = cell_center_world(r, c, tileScale);

                if (glm::length2(center - playerPos) < 10.0f * 10.0f)
                    continue;

                if ((r + c) % 4 == 0)
                {
                    enemies.push_back({EnemyType::CylinderBot, center, glm::vec3(0.0f), 0.9f, 1.6f, 90});
                }
                else if ((r + c) % 4 == 1)
                {
                    Enemy drone;
                    drone.type      = EnemyType::SphereDrone;
                    drone.pos       = center + glm::vec3(0.0f, 3.5f, 0.0f);
                    drone.radius    = 0.9f;
                    drone.height    = 1.0f;
                    drone.health    = 60;
                    drone.hoverBase = drone.pos.y;
                    enemies.push_back(drone);
                }
                else if ((r + c) % 4 == 2)
                {
                    Enemy turret;
                    turret.type   = EnemyType::CubeTurret;
                    turret.pos    = center;
                    turret.radius = 0.8f;
                    turret.health = 80;
                    enemies.push_back(turret);
                }
                else if ((r + c) % 4 == 3)
                {
                    Enemy charger;
                    charger.type        = EnemyType::PrismCharger;
                    charger.pos         = center;
                    charger.radius      = 0.9f;
                    charger.height      = 1.2f;
                    charger.health      = 70;
                    charger.chargeState = PrismChargeState::Idle;
                    enemies.push_back(charger);
                }
            }
        }
    }

    void update_enemies(std::vector<Enemy> & enemies,
                        std::vector<Bullet> & bullets,
                        float dt,
                        const Maze & maze,
                        const PlayerMovement & player,
                        float tileScale,
                        float globalTime)
    {
        const glm::vec3 playerPos = player.position;
        const float playerGround  = player.groundHeight;

        for (Enemy & e : enemies)
        {
            e.fireCooldown = std::max(0.0f, e.fireCooldown - dt);

            glm::vec3 toPlayer = playerPos - e.pos;
            float distance2    = glm::length2(toPlayer);
            glm::vec3 dir      = (distance2 > 0.0f)
                ? (toPlayer / std::sqrt(distance2))
                : glm::vec3(0.0f, 0.0f, 1.0f);

            switch (e.type)
            {
                case EnemyType::CylinderBot:
                {
                    e.pos.y = playerGround;
                    float chaseRange = 30.0f;
                    float fireRange  = 28.0f;
                    if (distance2 < chaseRange * chaseRange)
                    {
                        glm::vec3 proposed = e.pos + dir * CYLINDER_SPEED * dt;
                        if (!collides_with_wall(maze, proposed.x, proposed.z, e.radius, tileScale))
                            e.pos = proposed;
                    }

                    if (distance2 < fireRange * fireRange && e.fireCooldown <= 0.0f)
                    {
                        Bullet b;
                        b.fromPlayer = false;
                        b.radius     = 0.25f;
                        b.pos        = e.pos + glm::vec3(0.0f, e.height * 0.5f, 0.0f) + dir * (e.radius + b.radius + 0.2f);
                        b.vel        = glm::normalize(glm::vec3(dir.x, 0.1f, dir.z)) * BULLET_SPEED;
                        bullets.push_back(b);
                        e.fireCooldown = 1.3f;
                    }
                    break;
                }
                case EnemyType::SphereDrone:
                {
                    e.pos.y = e.hoverBase + std::sin(globalTime * 2.0f) * 0.5f;
                    glm::vec3 horizontal(dir.x, 0.0f, dir.z);
                    float fireRange = 32.0f;
                    if (glm::length2(horizontal) > 0.0f)
                    {
                        e.pos += glm::normalize(horizontal) * DRONE_SPEED * dt;
                        e.pos  = clamp_to_maze(e.pos, tileScale, maze);
                    }

                    if (distance2 < fireRange * fireRange && e.fireCooldown <= 0.0f)
                    {
                        Bullet b;
                        b.fromPlayer = false;
                        b.radius     = 0.25f;
                        b.pos        = e.pos + glm::vec3(0.0f, 0.2f, 0.0f) + dir * (e.radius + b.radius + 0.2f);
                        b.vel        = glm::normalize(dir) * BULLET_SPEED;
                        bullets.push_back(b);
                        e.fireCooldown = 1.0f;
                    }
                    break;
                }
                case EnemyType::CubeTurret:
                {
                    e.pos.y = playerGround;
                    float fireRange = 35.0f;
                    if (distance2 < fireRange * fireRange && e.fireCooldown <= 0.0f)
                    {
                        bool blocked = false;
                        glm::vec3 step = dir * 1.0f;
                        glm::vec3 probe = e.pos;
                        for (int s = 0; s < static_cast<int>(fireRange); ++s)
                        {
                            probe += step;
                            if (collides_with_wall(maze, probe.x, probe.z, 0.4f, tileScale))
                            {
                                blocked = true;
                                break;
                            }
                        }

                        if (!blocked)
                        {
                            Bullet b;
                            b.fromPlayer = false;
                            b.radius     = 0.28f;
                            b.pos        = e.pos + glm::vec3(0.0f, e.height * 0.5f, 0.0f) + dir * (e.radius + b.radius + 0.1f);
                            b.vel        = glm::normalize(dir) * BULLET_SPEED;
                            bullets.push_back(b);
                            e.fireCooldown = 1.6f;
                        }
                    }
                    break;
                }
                case EnemyType::PrismCharger:
                {
                    e.pos.y = playerGround;
                    float trigger = 20.0f;
                    if (e.chargeState == PrismChargeState::Idle && distance2 < trigger * trigger)
                    {
                        e.chargeState = PrismChargeState::Charging;
                        e.stateTimer  = CHARGER_DURATION;
                        e.vel         = dir * CHARGER_SPEED;
                    }

                    if (e.chargeState == PrismChargeState::Charging)
                    {
                        e.stateTimer -= dt;
                        glm::vec3 proposed = e.pos + e.vel * dt;
                        if (!collides_with_wall(maze, proposed.x, proposed.z, e.radius, tileScale))
                            e.pos = proposed;
                        if (e.stateTimer <= 0.0f)
                        {
                            e.chargeState = PrismChargeState::Idle;
                            e.vel         = glm::vec3(0.0f);
                        }
                    }
                    break;
                }
            }
        }
    }

    void update_bullets(std::vector<Bullet> & bullets,
                        std::vector<Enemy> & enemies,
                        float dt,
                        const Maze & maze,
                        PlayerMovement & player,
                        float tileScale)
    {
        for (size_t i = 0; i < bullets.size();)
        {
            Bullet & b = bullets[i];
            b.pos += b.vel * dt;
            b.ttl -= dt;

            bool removeBullet = (b.ttl <= 0.0f);

            if (!removeBullet && collides_with_wall(maze, b.pos.x, b.pos.z, b.radius, tileScale))
                removeBullet = true;

            if (!removeBullet && !b.fromPlayer)
            {
                float r = b.radius + game::PLAYER_RADIUS;
                if (glm::length2(b.pos - player.position) < r * r)
                {
                    player.health = std::max(0, player.health - static_cast<int>(ENEMY_BULLET_DAMAGE));
                    removeBullet = true;
                }
            }

            if (!removeBullet && b.fromPlayer)
            {
                for (Enemy & e : enemies)
                {
                    if (e.health <= 0)
                        continue;

                    float r = b.radius + e.radius;
                    if (glm::length2(b.pos - e.pos) < r * r)
                    {
                        e.health -= static_cast<int>(PLAYER_BULLET_DAMAGE);
                        removeBullet = true;
                        break;
                    }
                }
            }

            if (removeBullet)
            {
                bullets[i] = bullets.back();
                bullets.pop_back();
            }
            else
            {
                ++i;
            }
        }

        for (Enemy & e : enemies)
        {
            if (e.type == EnemyType::PrismCharger && e.chargeState == PrismChargeState::Charging)
            {
                float r = e.radius + game::PLAYER_RADIUS;
                if (glm::length2(e.pos - player.position) < r * r)
                {
                    player.health = std::max(0, player.health - static_cast<int>(CHARGER_MELEE_DAMAGE));
                    e.chargeState = PrismChargeState::Idle;
                    e.vel = glm::vec3(0.0f);
                }
            }
        }

        enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
            [](const Enemy & e){ return e.health <= 0; }), enemies.end());
    }

    void draw_enemies(const std::vector<Enemy> & enemies)
    {
        for (const Enemy & e : enemies)
        {
            glPushMatrix();
            glTranslatef(e.pos.x, e.pos.y + e.height * 0.5f, e.pos.z);

            switch (e.type)
            {
                case EnemyType::CylinderBot:
                    glColor3f(0.2f, 0.6f, 0.2f);
                    draw_cylinder(e.radius, e.height);
                    break;
                case EnemyType::SphereDrone:
                    glColor3f(0.2f, 0.4f, 1.0f);
                    draw_sphere(e.radius, 14, 18);
                    break;
                case EnemyType::CubeTurret:
                    glColor3f(0.6f, 0.2f, 0.2f);
                    draw_cube(e.radius * 2.0f);
                    break;
                case EnemyType::PrismCharger:
                    glColor3f(0.9f, 0.9f, 0.1f);
                    if (glm::length2(e.vel) > 0.01f)
                    {
                        float yaw = std::atan2(e.vel.z, e.vel.x) * 180.0f / static_cast<float>(M_PI);
                        glRotatef(yaw, 0.0f, 1.0f, 0.0f);
                    }
                    draw_prism(e.height * 1.5f, e.radius);
                    break;
            }

            glPopMatrix();
        }
    }

    void draw_bullets(const std::vector<Bullet> & bullets)
    {
        for (const Bullet & b : bullets)
        {
            glPushMatrix();
            glTranslatef(b.pos.x, b.pos.y, b.pos.z);
            if (b.fromPlayer)
                glColor3f(1.0f, 0.8f, 0.2f);
            else
                glColor3f(0.2f, 0.2f, 0.2f);
            draw_sphere(b.radius, 10, 12);
            glPopMatrix();
        }
    }
}
