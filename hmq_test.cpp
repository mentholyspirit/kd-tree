#include <cstdio>
#include "ply_reader.h"
#include "vector3.h"
#include "triangle.h"

int main(int argv, char** argc)
{
    Vector3 asd;
    asd.x = argv;
    Triangle t;
    t.vertices[0] = asd;
    std::unique_ptr<PLY_Model> model = Read_PLY_Model("happy_vrip_res4.ply");
    t.vertices[0].x = model->Get_Coordinate(0, 1, 0);
    printf("ASD:%f\n", t.vertices[0].x);
    return 0;
}
