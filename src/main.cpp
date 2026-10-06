#include "raytracer.h"
#include <exception>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: render_scene scene.obj output.png\n"
                     "Default camera: (-0.5, 1.5, 0.98) looking at (0, 1, 0).\n";
        return 1;
    }
    try {
        CameraOptions camera{.screen_width = 500, .screen_height = 500,
                             .look_from = {-0.5, 1.5, 0.98}, .look_to = {0., 1., 0.}};
        auto image = Render(argv[1], camera, RenderOptions{4});
        image.Write(argv[2]);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
