#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>
#include "Actor.h"

#include <vector>

class Maze;

namespace mygllib
{
    class GLFWInput;
    class View;
}

namespace game
{
    const float PLAYER_BODY_HEIGHT = 1.4f;
    const float PLAYER_EYE_HEIGHT  = PLAYER_BODY_HEIGHT;
    const float PLAYER_RADIUS      = 0.8f;
    const float PLAYER_EYE_RADIUS  = PLAYER_RADIUS + 0.08f;

    struct PlayerInput
    {
    };

    struct PlayerMovement
    {
        glm::vec3 position{0.0f};
        glm::vec3 velocity{0.0f};
        glm::vec3 facing_direction{0.0f, 0.0f, -1.0f};
        int health{100};
        int max_health{100};
        int score{0};
        bool on_ground{true};
        bool dashing{false};
        bool sliding{false};
        float dash_timer{0.0f};
        float slide_timer{0.0f};

        // Movement tuning parameters
        float max_ground_speed   = 15.0f;
        float max_air_speed      = 25.0f;
        float accel_ground       = 80.0f;
        float accel_air          = 40.0f;
        float friction_ground    = 10.0f;
        float friction_air       = 0.5f;
        float slide_friction     = 2.0f;
        float jump_speed         = 10.0f;
        float gravity          = 25.0f;
        float dash_speed        = 40.0f;
        float dash_duration     = 0.12f;
        float slide_duration    = 0.5f;
        float slide_threshold   = 8.0f;
        float collision_radius  = PLAYER_EYE_RADIUS + 0.3f;   // horizontal collision radius in world units
        float fire_rate         = 0.2f;    // seconds between shots
        float damage_buffer     = 0.0f;    // accumulates fractional damage before applying to integer health

        // Utility state
        float ground_height     = 0.0f;
        bool initialized       = false;
        bool dash_key_last       = false;
        bool jump_key_last       = false;
        bool crouch_key_last     = false;
        float fire_cooldown      = 0.0f;
    };

    class Player : public Actor
    {
    public:
        bool on_ground{false};
        float fire_cooldown{0.0f};

        void update(const Maze & maze, const PlayerInput & input, float dt);
    };

    // Persistent singleton access to the player movement state
    PlayerMovement & player_movement_state();

    // Main movement update that applies input-driven acceleration, friction, dash, and slide.
    void update_player_movement(const mygllib::GLFWInput & input,
                                float dt,
                                mygllib::View & view,
                                const Maze & maze,
                                float tile_scale);

}

#endif // PLAYER_H
