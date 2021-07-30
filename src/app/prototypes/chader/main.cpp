#include "Scene.h"

#include <engine/Application.h>
#include <engine/Engine.h>
#include <engine/Timer.h>


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
