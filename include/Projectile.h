#ifndef PROJECTILE_H
#define PROJECTILE_H

#pragma once

#include "myglm.h"

struct Projectile
{
    glm::vec3 pos;
    glm::vec3 vel;
    float radius;
    bool alive;
};

#endif // PROJECTILE_H
