#include <cstdio>
#include "ply_reader.h"
#include "raytracer.h"
#include "tga_saver.h"

int main()
{
    uint16_t width = 640, height = 480;
    std::unique_ptr<PLY_Model> model = Read_PLY_Model("happy_vrip.ply");
    Raytracer raytracer;
    raytracer.SetModel(model.get());
    raytracer.SetResolution(width, height);
    raytracer.SetFOV((float)M_PI / 6.0f);
    raytracer.SetCameraPosition(Vector3(0.0f, 0.15f, 0.5f));
    raytracer.Setup();
    printf("Testing pixels...\n");
    for (int y = 100; y < 200; ++y)
    {
        raytracer.SetUseKDTree(false);
        Color c1 = raytracer.GetPixel(320, y);
        raytracer.SetUseKDTree(true);
        Color c2 = raytracer.GetPixel(320, y);
        assert(c1.r == c2.r);
        assert(c1.g == c2.g);
        assert(c1.b == c2.b);
    }
    printf("\x1b[32m[Test Passed]\n");
    return 0;
}
