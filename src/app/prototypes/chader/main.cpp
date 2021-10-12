#include "Scene.h"

#include <graphics/ApplicationGlfw.h>
#include <graphics/AppInterface.h>
#include <graphics/Timer.h>


using namespace ad;
using namespace ad::graphics;

int main(int argc, const char * argv[])
{
    ApplicationGlfw application("Chader", 800, 600);

    ad::graphics::Timer timer{glfwGetTime(), 0.};

    Scene scene(argv);

    while(application.nextFrame())
    {
        scene.step();
        timer.mark(glfwGetTime());
    }

    std::exit(EXIT_SUCCESS);
}
