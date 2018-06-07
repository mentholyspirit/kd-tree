#include "raytracer.h"
#include "triangle.h"
#include <chrono>

struct Ray
{
    Vector3 origin;
    Vector3 direction;
};

//[Möller-Trumbore] http://www.graphics.cornell.edu/pubs/1997/MT97.pdf
static inline bool TestTriangle(const Triangle& triangle, const Ray& ray)
{
    Vector3 edge1 = triangle.vertices[1] - triangle.vertices[0];
    Vector3 edge2 = triangle.vertices[2] - triangle.vertices[0];
    Vector3 pvec = Vector3::Cross(ray.direction, edge2);
    float det = Vector3::Dot(edge1, pvec);
    //if (det > -0.000001f && det < 0.000001f)
    //  return false;
    float inv_det = 1.0f / det;
    Vector3 tvec = ray.origin - triangle.vertices[0];
    float u = Vector3::Dot(tvec, pvec) * inv_det;
    if (u < 0.0f || u > 1.0f)
        return false;
    Vector3 qvec = Vector3::Cross(tvec, edge1);
    float v = Vector3::Dot(ray.direction, qvec) * inv_det;
    int a = 0;
    if (v < 0.0f || u + v >= 1.0f)
        return false;
    return true;
}

static inline bool TestTriangles(const std::vector<Triangle>& triangles, const Ray& ray, int* outTriangleIdx)
{
    //printf("Ray:(%f,%f,%f)->(%f,%f,%f)\n", ray.origin.x, ray.origin.y, ray.origin.z, ray.direction.x, ray.direction.y, ray.direction.z);
    //printf("count:%i\n", triangles.size());
        std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::high_resolution_clock::now();
        bool b = 0;
        for (int i = 0; i < triangles.size(); ++i)
        {
            bool a = TestTriangle(triangles[i], ray);
            if (a)
            {
                b = 1;
                *outTriangleIdx = i;
                //printf("hit");
                //return true;
            }
        }

        std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        //printf("Took:%f\n", duration * 1000000);

        if (b)
            return true;
    return false;
}

Color Raytracer::GetPixel(uint16_t x, uint16_t y)
{
    float inverseWidth = 1.0f / (float)m_ResolutionX;
    float inverseHeight = 1.0f / (float)m_ResolutionY;
    float aspectRatio = m_ResolutionX * inverseHeight;
    float angle = tan(m_FOV * 0.5);
//printf("angle%i", m_ResolutionX);

    float invWidth = inverseWidth;
    float invHeight = inverseHeight;
    float angle_aspectratio = angle * aspectRatio;
    float invWidth_angle_aspectratio = inverseWidth * angle_aspectratio;
    float invWidth2_angle_aspectratio = 2 * inverseWidth * angle_aspectratio; // 2 * inverseWidth * angle_aspectratio
    float invWidth_angle_aspectratio_angle_aspectratio = invWidth_angle_aspectratio - angle_aspectratio;
    float rayX = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectRatio;
    //float xx = (2 * (x * invWidth + 0.5 * invWidth) - 1) * angle_aspectratio;
    //float xx = ((2* x * invWidth + invWidth) - 1) * angle_aspectratio;
    //float xx = ((x * invWidth2 + invWidth) - 1) * angle_aspectratio;
    //float xx = x * invWidth2_angle_aspectratio + invWidth_angle_aspectratio - angle_aspectratio;
    float xx2 = x * invWidth2_angle_aspectratio + invWidth_angle_aspectratio_angle_aspectratio;
    
    float rayY = (2 * ((y + 0.5) * invHeight)) * angle;

    Ray ray;
    ray.origin = m_CameraPosition;
    ray.direction = Vector3(rayX, rayY, -1).Normalized();
    int triangleIdx;
    if (TestTriangles(m_Model->triangles, ray, &triangleIdx))
    {
        Vector3 n = m_Model->triangleNormals[triangleIdx];
        n = Vector3(0.5f, 0.5f, 0.5f) * Vector3::Dot(n, Vector3(0.0f, 1.0f, 0.0f)) + Vector3(0.5f, 0.5f, 0.5f);
        //Vector3 n = m_Model->triangleNormals[triangleIdx] * 0.5f + Vector3(0.5f, 0.5f, 0.5f);
        return { uint8_t(std::clamp(n.z, 0.0f, 1.0f) * 255), uint8_t(std::clamp(n.y, 0.0f, 1.0f) * 255), uint8_t(std::clamp(n.x, 0.0f, 1.0f) * 255) };
        //return { uint8_t(n.z * 255), uint8_t(n.y * 255), uint8_t(n.x * 255)};
        //return { 255, 255, 255 };
    }
    else
    {
        return { 0, 0, 0 };
    }
}

std::vector<Color> Raytracer::Trace()
{
    std::vector<Color> pixels;
    pixels.reserve(m_ResolutionX * m_ResolutionY);
    for (int y = 0; y < m_ResolutionY; ++y)
    {
        printf("%i\n", y);
        for (int x = 0; x < m_ResolutionX; ++x)
        {
            pixels.push_back(GetPixel(x, y));
        }
    }
    return pixels;
}