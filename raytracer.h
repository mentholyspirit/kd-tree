#pragma once
#include <math.h>
#include "ply_reader.h"

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
        float fov = M_PI / 6,
        Vector3 cameraPosition = Vector3(0, 0, 0),
        Vector3 forward = Vector3(0, 1, 0))
    {
        m_Model = model;
        m_ResolutionX = resolutionX;
        m_ResolutionY = resolutionY;
        m_FOV = fov;
        m_CameraPosition = cameraPosition;
        m_Forward = forward;
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
        m_Forward = forward;
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

    Color GetPixel(uint16_t x, uint16_t y);

    std::vector<Color> Trace();

private:
    PLY_Model* m_Model;
    uint16_t m_ResolutionX;
    uint16_t m_ResolutionY;
    float m_FOV;
    Vector3 m_CameraPosition;
    Vector3 m_Forward;
};