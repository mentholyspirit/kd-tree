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
    if (v < 0.0f || u + v >= 1.0f)
        return false;
    *outT = Vector3::Dot(edge2, qvec) * inv_det;
    return true;
}

static inline const Triangle* TestTriangles(const std::vector<Triangle>& triangles, const Ray& ray, float* outDistSq)
{
    *outDistSq = std::numeric_limits<float>::max();
    const Triangle* result = nullptr;
    for (int i = 0; i < triangles.size(); ++i)
    {
        float t;
        if (TestTriangle(triangles[i], ray, &t) && t < *outDistSq)
        {
            result = &triangles[i];
            *outDistSq = t;
        }
    }
    return result;
}

//traverse through nodes in the KDTree, return the closest triangle
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

static inline Color GetPixelInternal(const std::vector<Triangle>& triangles, Vector3 cameraPosition, float rayX, float rayY, const KDTree* kdTree = nullptr)
{
    Ray ray = Ray(cameraPosition, Vector3(rayX, rayY, -1).Normalized());
    const Triangle* triangle = nullptr;
    if (kdTree != nullptr)
    {
        triangle = Travese(ray, kdTree->GetRoot());
    }
    else
    {
        float outDistSq;
        triangle = TestTriangles(triangles, ray, &outDistSq);
    }
    if (triangle)
    {
        // "shading"
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
    float fovTan = tan(m_FOV * 0.5f);
    float rayX = (2 * (x * inverseWidth) - 1) * fovTan * aspectRatio;
    float rayY = (2 * (y * inverseHeight) - 1) * fovTan;
    return GetPixelInternal(m_Model->triangles, m_CameraPosition, rayX, rayY, m_UseKDTree ? m_KDTree.get() : nullptr);
}

std::vector<Color> Raytracer::Trace() const
{
    printf("Tracing pixels...\n");
    std::vector<Color> pixels;
    pixels.reserve(m_ResolutionX * m_ResolutionY);
    for (uint16_t y = 0; y < m_ResolutionY; ++y)
    {
        for (uint16_t x = 0; x < m_ResolutionX; ++x)
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
    printf("Creating KDTree...\n");
    m_KDTree = std::make_unique<KDTree>(m_Model->triangles, aabb);
}
