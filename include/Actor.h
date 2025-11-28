#pragma once

#include "myglm.h"

class Actor
{
public:
    Actor();
    virtual ~Actor();

    virtual void update(float dt);

    glm::vec3 pos;
    float yaw;
    float radius;
    float height;
    int   health;
};

