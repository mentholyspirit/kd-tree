#include "aabb.h"
#include "ray.h"

bool AABB::Intersects(const Ray& ray) const
{
    float tmin = (min.x - ray.origin.x) * ray.inverseDirection.x;
    float tmax = (max.x - ray.origin.x) * ray.inverseDirection.x;
    if (tmin > tmax)
        std::swap(tmin, tmax);

    float ymin = (min.y - ray.origin.y) * ray.inverseDirection.y;
    float ymax = (max.y - ray.origin.y) * ray.inverseDirection.y;
    if (ymin > ymax)
        std::swap(ymin, ymax);
    if (tmin > ymax || ymin > tmax)
        return false;

    if (ymin > tmin)
        tmin = ymin;
    if (ymax < tmax)
        tmax = ymax;

    float zmin = (min.z - ray.origin.z) * ray.inverseDirection.z;
    float zmax = (max.z - ray.origin.z) * ray.inverseDirection.z;
    if (zmin > zmax)
        std::swap(zmin, zmax);
    if (tmin > zmax || zmin > tmax)
        return false;

    return true;
}
