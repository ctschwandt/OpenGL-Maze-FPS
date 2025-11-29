#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>

#include "Actor.h"

class Maze;

namespace mygllib
{
    class GLFWInput;
    class View;
}

namespace game
{
    constexpr float PLAYER_BODY_HEIGHT = 1.4f;
    constexpr float PLAYER_EYE_HEIGHT  = PLAYER_BODY_HEIGHT;
    constexpr float PLAYER_RADIUS      = 0.8f;
    constexpr float PLAYER_EYE_RADIUS  = PLAYER_RADIUS + 0.08f;

    struct PlayerInput
    {
    };

    struct PlayerMovement
    {
        glm::vec3 position{0.0f};
        glm::vec3 velocity{0.0f};
        bool onGround{true};
        bool dashing{false};
        bool sliding{false};
        float dashTimer{0.0f};
        float slideTimer{0.0f};

        // Combat
        int   health{100};
        float fireCooldown{0.0f};

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
        float collisionRadius  = PLAYER_EYE_RADIUS + 0.3f;   // horizontal collision radius in world units

        // Utility state
        float groundHeight     = 0.0f;
        bool initialized       = false;
        bool dashKeyLast       = false;
        bool jumpKeyLast       = false;
        bool crouchKeyLast     = false;
    };

    class Player : public Actor
    {
    public:
        bool onGround{false};
        float fireCooldown{0.0f};

        void update(const Maze & maze, const PlayerInput & input, float dt);
    };

    // Persistent singleton access to the player movement state
    PlayerMovement & player_movement_state();

    // Main movement update that applies input-driven acceleration, friction, dash, and slide.
    void update_player_movement(const mygllib::GLFWInput & input,
                                float dt,
                                mygllib::View & view,
                                const Maze & maze,
                                float tileScale);
}

#endif // PLAYER_H
