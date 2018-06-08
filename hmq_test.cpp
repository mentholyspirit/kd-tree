#include <cstdio>
#include "ply_reader.h"
#include "raytracer.h"
#include "tga_saver.h"

int main(int argv, char** argc)
{
    uint16_t width = 640, height = 480;
    std::unique_ptr<PLY_Model> model = Read_PLY_Model("happy_vrip_res4.ply");
    //std::unique_ptr<PLY_Model> model = Read_PLY_Model("happy_vrip.ply");
    Raytracer raytracer;
    raytracer.SetModel(model.get());
    raytracer.SetResolution(width, height);
    raytracer.SetFOV((float)M_PI / 6.0f);
    raytracer.SetCameraPosition(Vector3(0.0f, 0.15f, 0.5f));
    raytracer.Setup();
    raytracer.SetUseKDTree(!true);
    std::vector<Color> image = raytracer.Trace();
    Write_Tga("image.tga", width, height, image.data());
    return 0;
}
