#include <cstdio>
#include "ply_reader.h"
#include "vector3.h"
#include "triangle.h"
#include "raytracer.h"
#include "tga_saver.h"

int main(int argv, char** argc)
{
    int width = 640, height = 480;
    //std::unique_ptr<PLY_Model> model = Read_PLY_Model("happy_vrip_res4.ply");
    std::unique_ptr<PLY_Model> model = Read_PLY_Model("happy_vrip.ply");
    Raytracer raytracer;
    raytracer.SetModel(model.get());
    raytracer.SetResolution(width, height);
    raytracer.SetFOV(M_PI / 6);
    raytracer.SetCameraPosition(Vector3(0.0f, 0.15f, 0.5f));
    raytracer.Setup();
    std::vector<Color> image = raytracer.Trace();
    Write_Tga("image.tga", width, height, image.data());
    return 0;
}
