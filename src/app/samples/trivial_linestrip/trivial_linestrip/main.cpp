#include "Scene.h"

#include <graphics/Application.h>
#include <graphics/Engine.h>
#include <graphics/Timer.h>


using namespace ad;

int main(int argc, const char * argv[])
{
    try
    {
        Application application("Trivial Line Strip", 800, 600);

        ad::Timer timer{glfwGetTime(), 0.};

        Scene scene{ {200, 150} };

        while(application.nextFrame())
        {
            timer.mark(glfwGetTime());
            scene.step(timer);
            application.getEngine()->clear();
            scene.render();
        }
    }
    catch(const std::exception & e)
    {
        std::cerr << "Exception:\n"
                  << e.what()
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::exit(EXIT_SUCCESS);
}
