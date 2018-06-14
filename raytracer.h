#pragma once
#include <math.h>
#include "ply_reader.h"
#include "kdtree.h"

struct Color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class Raytracer
{
public:
    Raytracer(
        PLY_Model* model = nullptr,
        uint16_t resolutionX = 640,
        uint16_t resolutionY = 480,
        float fov = (float)M_PI / 6.0f,
        Vector3 cameraPosition = Vector3(0, 0, 0),
        Vector3 forward = Vector3(0, 0, 1))
    {
        m_Model = model;
        m_ResolutionX = resolutionX;
        m_ResolutionY = resolutionY;
        m_FOV = fov;
        m_CameraPosition = cameraPosition;
        m_Forward = forward.Normalized();
    }

    void SetModel(PLY_Model* model)
    {
        m_Model = model;
    }

    void SetCameraPosition(Vector3 cameraPosition)
    {
        m_CameraPosition = cameraPosition;
    }

    void SetForward(Vector3 forward)
    {
        m_Forward = forward.Normalized();
        m_Left = Vector3::Cross(m_Forward, Vector3(0, 1, 0)).Normalized();
        m_Down = Vector3::Cross(m_Forward, m_Left).Normalized();
    }

    void SetResolution(uint16_t resolutionX, uint16_t resolutionY)
    {
        m_ResolutionX = resolutionX;
        m_ResolutionY = resolutionY;
    }

    void SetFOV(float fov)
    {
        m_FOV = fov;
    }

    void SetUseKDTree(bool useKDTree)
    {
        m_UseKDTree = useKDTree;
    }

    void SetSkybox(uint8_t* data, uint16_t width, uint16_t height)
    {
        m_Skybox = data;
        m_SkyboxWidth = width;
        m_SkyboxHeight = height;
    }

    void Setup();

    Color GetPixel(uint16_t x, uint16_t y) const;

    std::vector<Color> Trace() const;

private:
    PLY_Model* m_Model;
    uint16_t m_ResolutionX;
    uint16_t m_ResolutionY;
    float m_FOV;
    Vector3 m_CameraPosition;
    Vector3 m_Forward;
    Vector3 m_Left;
    Vector3 m_Down;
    std::unique_ptr<KDTree> m_KDTree;
    bool m_UseKDTree;
    uint8_t* m_Skybox;
    uint16_t m_SkyboxWidth;
    uint16_t m_SkyboxHeight;
};
