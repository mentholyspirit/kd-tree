#include "raytracer.h"
#include "triangle.h"
#include "ray.h"
#include <chrono>
#include <pthread.h>

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

static inline Color GetPixelInternal(const std::vector<Triangle>& triangles, Vector3 cameraPosition, Vector3 rayDir, const KDTree* kdTree = nullptr)
{
    Ray ray = Ray(cameraPosition, rayDir.Normalized());
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
        /*
        n = Vector3(0.05f, 0.1f, 0.1f) * Vector3::Dot(n, Vector3(1.0f, 0.0f, 0.0f)) * 1.0f + Vector3(0.5f, 0.1f, 0.1f) * 0.2f;
        */

        uint8_t o[3] = { uint8_t(std::clamp(n.x, 0.0f, 1.0f) * 255), uint8_t(std::clamp(n.y, 0.0f, 1.0f) * 255), uint8_t(std::clamp(n.z, 0.0f, 1.0f) * 255) };


        float cosi = std::clamp(-1.0f, 1.0f, Vector3::Dot(ray.direction, n)); 
        float etai = 1, etat = 0.90; 
        Vector3 m = n;
        if (cosi < 0) { cosi = -cosi; } else { std::swap(etai, etat); m= n * -1.0f; } 
        float eta = etai / etat; 
        float k = 1 - eta * eta * (1 - cosi * cosi); 
        Vector3 l = k < 0.0f ? Vector3(0.0f, 0.0f, 0.0f) : ray.direction * eta + m * (eta * cosi - sqrtf(k));

        Vector3 bgHit = l.Normalized();
        bool p = (uint8_t(atan2(bgHit.z, bgHit.x) * 33 + 20) % 2) != (uint8_t(bgHit.y * 33 + 20) % 2);
        return { uint8_t(p * 150 + o[0]), uint8_t(p * 150 + o[1]), uint8_t(150 + o[2]) };
    }
    else
    {
        Vector3 bgHit = rayDir.Normalized();
        bool p = (uint8_t(atan2(bgHit.z, bgHit.x) * 33 + 20) % 2) != (uint8_t(bgHit.y * 33 + 20) % 2);
        return { uint8_t(p * 110 + 110), uint8_t(p * 110 + 110), uint8_t(110) };
        //return { 110, 110, 110 };
    }
}

Color Raytracer::GetPixel(uint16_t x, uint16_t y) const
{
    float inverseWidth = 1.0f / (float)m_ResolutionX;
    float inverseHeight = 1.0f / (float)m_ResolutionY;
    float aspectRatio = m_ResolutionX * inverseHeight;
    float fovTan = tan(m_FOV * 0.5f);
    Vector3 L = m_Left * ((2 * (x * inverseWidth) - 1) * fovTan * aspectRatio);
    Vector3 D = m_Down * ((2 * (y * inverseHeight) - 1) * fovTan);
    return GetPixelInternal(m_Model->triangles, m_CameraPosition, L + D + m_Forward, m_UseKDTree ? m_KDTree.get() : nullptr);
}

struct TraceThreadArgs
{
    int index;
    const Raytracer* raytracer;
};

#define NUM_THREADS 16
#define THREAD_STRIDE (480 / NUM_THREADS)

static std::vector<Color> pixelArrays[NUM_THREADS];
static void* TraceThread(void* _args)
{
    TraceThreadArgs args = *(TraceThreadArgs*)_args;
    int yOff = args.index * THREAD_STRIDE;
    std::vector<Color> pixels;
    pixels.reserve(640 * THREAD_STRIDE);
    for (uint16_t y = yOff; y < yOff + THREAD_STRIDE; ++y)
    {
        for (uint16_t x = 0; x < 640; ++x)
        {
            pixels.push_back(args.raytracer->GetPixel(x, y));
        }
    }
    pixelArrays[args.index] = pixels;
    return nullptr;
}

std::vector<Color> Raytracer::Trace() const
{
    pthread_t threads[NUM_THREADS];
    TraceThreadArgs args[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++)
    {
        args[i].index = i;
        args[i].raytracer = this;
        
        pthread_create(&threads[i], nullptr, TraceThread, &args[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++)
    {
        if (pthread_join(threads[i], NULL))
        {
           printf("o\n");
        }
    }
    std::vector<Color> pixels;
    pixels.reserve(m_ResolutionX * m_ResolutionY);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pixels.insert(pixels.end(), pixelArrays[i].begin(), pixelArrays[i].end());
    }
    return pixels;
}

/*std::vector<Color> Raytracer::Trace() const
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
}*/

void Raytracer::Setup()
{
    AABB aabb;
    aabb.min = Vector3(-10, -10, -10);
    aabb.max = Vector3(10, 10, 10);
    printf("Creating kd-Tree...\n");
    m_KDTree = std::make_unique<KDTree>(m_Model->triangles, aabb);
}
