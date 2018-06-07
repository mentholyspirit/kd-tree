#pragma once
#include <algorithm>
#include "vector3.h"

enum Axis : unsigned char
{
    kAxisX,
    kAxisY,
    kAxisZ,
    kAxesCount
};

struct Triangle
{
    Vector3 vertices[3];

    Triangle(Vector3 a, Vector3 b, Vector3 c)
    {
        vertices[0] = a;
        vertices[1] = b;
        vertices[2] = c;
    }

    inline float GetAxisMin(Axis axis) const
    {
        return std::min(std::min(vertices[0][axis], vertices[1][axis]), vertices[2][axis]);
    }

    inline float GetAxisMax(Axis axis) const
    {
        return std::max(std::max(vertices[0][axis], vertices[1][axis]), vertices[2][axis]);
    }

    inline Vector3 GetNormal() const
    {
        Vector3 edge0 = vertices[1] - vertices[0];
        Vector3 edge1 = vertices[2] - vertices[0];
        return Vector3::Cross(edge0, edge1).Normalized();
    }
};
