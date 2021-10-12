#include "Scene.h"

#include <graphics/Application.h>
#include <graphics/AppInterface.h>
#include <graphics/Timer.h>


using namespace ad;

int main(int argc, const char * argv[])
{
    Application application("Chader", 800, 600);

    ad::Timer timer{glfwGetTime(), 0.};

    Scene scene(argv);

    while(application.nextFrame())
    {
        scene.step();
        timer.mark(glfwGetTime());
    }

    std::exit(EXIT_SUCCESS);
}
