#include "Enemy.h"

namespace game
{
    Enemy::Enemy(EnemyType type, float moveSpeed, float detectionRange, int baseHealth)
        : type_(type)
    {
        moveSpeed_ = moveSpeed;
        detectionRange_ = detectionRange;
        health = baseHealth;
    }

    CylinderBot::CylinderBot()
        : Enemy(EnemyType::CylinderBot, 6.0f, 12.0f, 80)
    {
        radius = 0.6f;
        height = 1.6f;
    }

    SphereDrone::SphereDrone()
        : Enemy(EnemyType::SphereDrone, 8.0f, 15.0f, 60)
    {
        radius = 0.7f;
        height = 1.0f;
    }

    CubeTurret::CubeTurret()
        : Enemy(EnemyType::CubeTurret, 0.0f, 20.0f, 120)
    {
        radius = 0.9f;
        height = 1.2f;
    }

    PyramidCharger::PyramidCharger()
        : Enemy(EnemyType::PyramidCharger, 10.0f, 10.0f, 90)
    {
        radius = 0.8f;
        height = 1.4f;
    }
}
