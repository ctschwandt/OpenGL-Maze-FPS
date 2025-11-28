#ifndef PLAYERMOVEMENT_H
#define PLAYERMOVEMENT_H

#include <glm/glm.hpp>

namespace mygllib
{
    class GLFWInput;
    class View;
}

namespace game
{
    struct PlayerMovement
    {
        glm::vec3 position{0.0f};
        glm::vec3 velocity{0.0f};
        bool onGround{true};
        bool dashing{false};
        bool sliding{false};
        float dashTimer{0.0f};
        float slideTimer{0.0f};

        // Movement tuning parameters
        float maxGroundSpeed   = 15.0f;
        float maxAirSpeed      = 25.0f;
        float accelGround      = 80.0f;
        float accelAir         = 40.0f;
        float frictionGround   = 10.0f;
        float frictionAir      = 0.5f;
        float slideFriction    = 2.0f;
        float jumpSpeed        = 10.0f;
        float gravity          = 25.0f;
        float dashSpeed        = 40.0f;
        float dashDuration     = 0.12f;
        float slideDuration    = 0.5f;
        float slideThreshold   = 8.0f;

        // Utility state
        float groundHeight     = 0.0f;
        bool initialized       = false;
        bool dashKeyLast       = false;
        bool jumpKeyLast       = false;
        bool crouchKeyLast     = false;
    };

    // Persistent singleton access to the player movement state
    PlayerMovement & player_movement_state();

    // Main movement update that applies input-driven acceleration, friction, dash, and slide.
    void update_player_movement(const mygllib::GLFWInput & input, float dt, mygllib::View & view);
}

#endif // PLAYERMOVEMENT_H

