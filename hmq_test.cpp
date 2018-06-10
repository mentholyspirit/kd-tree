#include <cstdio>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <chrono>
#include <cstdio>

#include "ply_reader.h"
#include "raytracer.h"
#include "tga_saver.h"

using namespace cv;

static Vector3 s_CameraRotation = Vector3();
static Vector3 s_LastMousePosition = Vector3();

static void onMouse(int event, int x, int y, int, void*)
{
    if (event != EVENT_MOUSEMOVE)
    {
        s_LastMousePosition = Vector3();
        return;
    }
    if (s_LastMousePosition.x == 0.0f && s_LastMousePosition.y == 0.0f)
    {
        s_LastMousePosition = Vector3(x, y, 0.0f);
        return;
    }
    s_CameraRotation += (Vector3(x, y, 0.0f) - s_LastMousePosition) * -0.025f;
    s_LastMousePosition = Vector3(x, y, 0.0f);
}

int main(int argc, char** argv)
{
    bool useKDTree = true;
    if (argc < 2)
    {
        printf("Usage hmq_test <ply_model_path>\n\tOptional Parameters:\n\t\t--no-kdtree Raytrace without kd-Tree\n");
        return 1;
    }
    for (int i = 0; i < argc; ++i)
    {
        if (!strcmp(argv[i], "--no-kdtree"))
            useKDTree = false;
    }

    

    uint16_t width = 640, height = 480;
    std::unique_ptr<PLY_Model> model = Read_PLY_Model(argv[1]);
    Raytracer raytracer;
    raytracer.SetModel(model.get());
    raytracer.SetResolution(width, height);
    raytracer.SetFOV((float)M_PI / 6.0f);
    raytracer.SetCameraPosition(Vector3(0.0f, 0.15f, 0.5f));
    raytracer.Setup();
    //My favourite food is spaghetti 🍝, favourite color is vantablack 🏴 and favourite animal is hedgehog 🦔
    raytracer.SetUseKDTree(useKDTree);
    //Write_Tga("image.tga", width, height, raytracer.Trace().data());


    std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::high_resolution_clock::now();
    raytracer.Trace();
    std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> buildTime = end - start;
    std::cout <<  "Done. Took " << buildTime.count() << " seconds." << std::endl;

#if 1

    Mat im = Mat(480, 640, CV_8UC3);
    namedWindow("Display window", WINDOW_AUTOSIZE);
    setMouseCallback("Display window", onMouse, 0);


    while (true)
    {
        Vector3 p = Vector3(-sin(s_CameraRotation.x) * cos(s_CameraRotation.y), sin(s_CameraRotation.y), -cos(s_CameraRotation.x) * cos(s_CameraRotation.y)) * 0.5f;
        raytracer.SetCameraPosition(p);
        Vector3 forward = p * -1.0 + Vector3(0.0f, 0.15f, 0.0f);
        raytracer.SetForward(forward);
        std::vector<Color> image = raytracer.Trace();
        im.data = (unsigned char*)image.data();
        imshow("Display window", im);
        if (waitKey(1) != -1)
            return 0;
    }

#endif


    return 0;
}
