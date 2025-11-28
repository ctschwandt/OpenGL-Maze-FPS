#pragma once

#include "myglm.h"

struct Projectile
{
    glm::vec3 pos;
    glm::vec3 vel;
    float radius;
    bool  alive;
};

