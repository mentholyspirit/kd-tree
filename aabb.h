#pragma once
#include "vector3.h"
#include "ray.h"

struct AABB
{
    Vector3 min;
    Vector3 max;
public:
    bool Intersects(const Ray& ray) const;
};
