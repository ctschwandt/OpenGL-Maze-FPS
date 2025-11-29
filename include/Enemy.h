#ifndef ENEMY_H
#define ENEMY_H

#include "Actor.h"

namespace game
{
    enum class EnemyType
    {
        CylinderBot,
        SphereDrone,
        CubeTurret,
        PyramidCharger
    };

    class Enemy : public Actor
    {
    public:
        Enemy(EnemyType type, float moveSpeed, float detectionRange, int baseHealth);
        virtual ~Enemy() = default;

        EnemyType type() const { return type_; }
        float move_speed() const { return moveSpeed_; }
        float detection_range() const { return detectionRange_; }

    protected:
        EnemyType type_;
        float moveSpeed_ = 0.0f;
        float detectionRange_ = 0.0f;
    };

    class CylinderBot : public Enemy
    {
    public:
        CylinderBot();
    };

    class SphereDrone : public Enemy
    {
    public:
        SphereDrone();
    };

    class CubeTurret : public Enemy
    {
    public:
        CubeTurret();
    };

    class PyramidCharger : public Enemy
    {
    public:
        PyramidCharger();
    };
}

#endif // ENEMY_H
