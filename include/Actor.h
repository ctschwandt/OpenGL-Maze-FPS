#pragma once

#include <glm/glm.hpp>

class Maze;

namespace game
{
    struct PlayerInput;

    class Actor
    {
    public:
        glm::vec3 pos{0.0f};   // world position (x, y, z)
        glm::vec3 vel{0.0f};   // world velocity (vx, vy, vz)
        float yaw{0.0f};       // horizontal orientation
        float radius{0.5f};    // collision radius in XZ
        float height{1.8f};    // vertical extent
        int health{100};

        virtual ~Actor() = default;
    };

    class Player : public Actor
    {
    public:
        bool onGround{false};
        float fireCooldown{0.0f};

        void update(const Maze & maze, const PlayerInput & input, float dt);
    };
}

