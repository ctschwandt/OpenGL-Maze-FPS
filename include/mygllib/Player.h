#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>

namespace mygllib
{
    struct Player
    {
        glm::vec3 pos{0.0f};
        glm::vec3 vel{0.0f};
        bool onGround{true};
        bool sliding{false};
    };
}

#endif // PLAYER_H

