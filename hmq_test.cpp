#include <cstdio>
#include "ply_reader.h"
#include "vector3.h"
#include "triangle.h"
#include "raytracer.h"
#include "tga_saver.h"

int main(int argv, char** argc)
{
    Vector3 asd;
    asd.x = argv;
    Triangle t((Vector3()), (Vector3()), (Vector3()));
    int width = 640, height = 480;
    t.vertices[0] = asd;
    std::unique_ptr<PLY_Model> model = Read_PLY_Model("happy_vrip_res4.ply");
    Raytracer raytracer;
    raytracer.SetModel(model.get());
    raytracer.SetResolution(width, height);
    //raytracer.GetPixel(5, 12);
    raytracer.SetCameraPosition(Vector3(0.0f, 0.0f, 0.5f));
    std::vector<Color> image = raytracer.Trace();
    Write_Tga("image.tga", width, height, image.data());

    //t.vertices[0].x = model->Get_Coordinate(0, 1, 0);
     printf("ASD:%lu\n", sizeof(Triangle));
    return 0;
}
