#pragma once
#include "vector3.h"

struct Triangle
{
    Vector3 vertices[3];

    Triangle(Vector3 a, Vector3 b, Vector3 c)
    {
        vertices[0] = a;
        vertices[1] = b;
        vertices[2] = c;
    }

    inline Vector3 GetNormal() const
    {
        Vector3 edge0 = vertices[1] - vertices[0];
        Vector3 edge1 = vertices[2] - vertices[0];
        return Vector3::Cross(edge0, edge1).Normalized();
    }
};
