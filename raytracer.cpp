#include "raytracer.h"
#include "triangle.h"
#include "ray.h"
#include <chrono>


//[Möller-Trumbore] http://www.graphics.cornell.edu/pubs/1997/MT97.pdf
static inline bool TestTriangle(const Triangle& triangle, const Ray& ray, float* outT)
{
    Vector3 edge1 = triangle.vertices[1] - triangle.vertices[0];
    Vector3 edge2 = triangle.vertices[2] - triangle.vertices[0];
    Vector3 pvec = Vector3::Cross(ray.direction, edge2);
    float det = Vector3::Dot(edge1, pvec);
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
    *outT = Vector3::Dot(edge2, qvec) * inv_det;
    return true;
}

static inline const Triangle* TestTriangles(const std::vector<Triangle>& triangles, const Ray& ray, float* outDistSq)
{
    *outDistSq = std::numeric_limits<float>::max();
    const Triangle* b = nullptr;
    for (int i = 0; i < triangles.size(); ++i)
    {
        float t;
        bool a = TestTriangle(triangles[i], ray, &t);
        if (a && t < *outDistSq)
        {
            b = &triangles[i];
            *outDistSq = t;
        }
    }
    return b;
}

static const Triangle* Travese(Ray& ray, const KDNode* node, float* outDistSq = nullptr)
{
    if (node->GetAABB().Intersects(ray))
    {
        const KDNode* left = node->GetLeft();
        const KDNode* right = node->GetRight();
        if (left || right)
        {
            float distL = std::numeric_limits<float>::infinity();
            float distR = std::numeric_limits<float>::infinity();
            const Triangle* leftTri = nullptr;
            const Triangle* rightTri = nullptr;
            if (left)
            {
                leftTri = Travese(ray, left, &distL);
            }
            if (right)
            {
                rightTri = Travese(ray, right, &distR);
            }
            if (leftTri && distL <= distR)
            {
                if (outDistSq)
                {
                    *outDistSq = distL;
                }
                return leftTri;
            }
            if (rightTri)
            {
                if (outDistSq)
                {
                    *outDistSq = distR;
                }
                return rightTri;
            }
        }
        else
        {
            return TestTriangles(node->GetTriangles(), ray, outDistSq);
        }
    }
    return nullptr;
}

static inline Color GetPixelInternal(Vector3 cameraPosition, KDTree* kdTree, float rayX, float rayY)
{
    Ray ray = Ray(cameraPosition, Vector3(rayX, rayY, -1).Normalized());
    const Triangle* triangle = nullptr;
    triangle = Travese(ray, kdTree->GetRoot());
    if (triangle)
    {
        Vector3 n = triangle->GetNormal();
        n = Vector3(0.5f, 1.0f, 1.0f) * Vector3::Dot(n, Vector3(1.0f, 0.0f, 0.0f)) * 0.8f + Vector3(0.1f, 0.4f, 0.5f) * 1.2f;
        return { uint8_t(std::clamp(n.x, 0.0f, 1.0f) * 255), uint8_t(std::clamp(n.y, 0.0f, 1.0f) * 255), uint8_t(std::clamp(n.z, 0.0f, 1.0f) * 255) };
    }
    else
    {
        return { 70, 0, 0 };
    }
}

Color Raytracer::GetPixel(uint16_t x, uint16_t y) const
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
    float rayX = (2 * (x * invWidth) - 1) * angle * aspectRatio;
    //float xx = (2 * (x * invWidth + 0.5 * invWidth) - 1) * angle_aspectratio;
    //float xx = ((2* x * invWidth + invWidth) - 1) * angle_aspectratio;
    //float xx = ((x * invWidth2 + invWidth) - 1) * angle_aspectratio;
    //float xx = x * invWidth2_angle_aspectratio + invWidth_angle_aspectratio - angle_aspectratio;
    float xx2 = x * invWidth2_angle_aspectratio + invWidth_angle_aspectratio_angle_aspectratio;
    
    float rayY = (2 * (y * invHeight) - 1) * angle;
    return GetPixelInternal(m_CameraPosition, m_KDTree.get(), rayX, rayY);   
}

std::vector<Color> Raytracer::Trace() const
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

void Raytracer::Setup()
{
    AABB aabb;
    aabb.min = Vector3(-10, -10, -10);
    aabb.max = Vector3(10, 10, 10);
    m_KDTree = std::make_unique<KDTree>(m_Model->triangles, aabb);
}
