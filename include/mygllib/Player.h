#pragma once

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

    extern Player player;

    class GLFWInput;

    // Update player physics and synchronize the view with the player's position.
    void update_player(const GLFWInput & input, float dt);
}

