#pragma once
#include <cmath>

struct Vector3
{
    float x;
    float y;
    float z;

    Vector3() {}
    Vector3(float x, float y, float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    inline float SqrMagnitude()
    {
        return x * x + y * y + z * z;
    }

    inline float Magnitude()
    {
        return std::sqrt(SqrMagnitude());
    }

    inline Vector3 Normalized()
    {
        return MultiplyScalar(*this, 1.0f / Magnitude());
    }

    inline Vector3 operator*(float other) const
    {
        return MultiplyScalar(*this, other);
    }

    inline Vector3 operator-(const Vector3& other) const
    {
        return Subtract(*this, other);
    }

    inline Vector3 operator+(const Vector3& other) const
    {
        return Add(*this, other);
    }

    inline Vector3 operator+=(const Vector3& other)
    {
        return *this = Add(*this, other);
    }

    inline static Vector3 MultiplyScalar(const Vector3& a, float s)
    {
        return Vector3(a.x * s, a.y * s, a.z * s);
    }

    inline static Vector3 Subtract(const Vector3& a, const Vector3& b)
    {
        return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
    }

    inline static Vector3 Add(const Vector3& a, const Vector3& b)
    {
        return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
    }

    inline static float Dot(const Vector3& a, const Vector3& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    inline static Vector3 Cross(const Vector3& a, const Vector3& b)
    {
        return Vector3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x);
    }

    inline const float& operator[](int index) const
    {
        return reinterpret_cast<const float*>(this)[index];
    }
    inline float& operator[](int index)
    {
        return reinterpret_cast<float*>(const_cast<Vector3*>(this))[index];
    }
};
