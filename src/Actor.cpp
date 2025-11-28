#include "Actor.h"

Actor::Actor()
    : pos(0.0f),
      yaw(0.0f),
      radius(0.3f),
      height(1.8f),
      health(100)
{
}

Actor::~Actor() = default;

void Actor::update(float dt)
{
    (void)dt;
}

