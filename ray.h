#pragma once
#include "vector3.h"

struct Ray
{
    Vector3 origin;
    Vector3 direction;
    Vector3 inverseDirection;
    Ray(Vector3 origin, Vector3 direction) :
        origin(origin),
        direction(direction),
        inverseDirection(Vector3(1.0f / direction.x, 1.0f / direction.y, 1.0f / direction.z))
    {}
    Ray() :
        origin(Vector3()),
        direction(Vector3()),
        inverseDirection(Vector3(0.0f, 0.0f, 0.0f))
    {}
};
