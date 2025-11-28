#ifndef ACTOR_H
#define ACTOR_H

#pragma once

#include "myglm.h"

class Actor
{
public:
    Actor();
    virtual ~Actor() = default;

    virtual void update(float dt);

    glm::vec3 pos;
    float yaw;
    float radius;
    float height;
    int health;
};

#endif // ACTOR_H
