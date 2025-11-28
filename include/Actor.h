#ifndef ACTOR_H
#define ACTOR_H

#include <glm/glm.hpp>

namespace game
{
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
}

#endif // ACTOR_H
