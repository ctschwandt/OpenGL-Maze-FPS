#ifndef PLAYER_H
#define PLAYER_H

#include "Actor.h"

class Maze;

namespace game
{
    struct PlayerInput;

    class Player : public Actor
    {
    public:
        bool onGround{false};
        float fireCooldown{0.0f};

        void update(const Maze & maze, const PlayerInput & input, float dt);
    };
}

#endif // PLAYER_H
