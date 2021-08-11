#include "Scene.h"

#include <engine/Application.h>
#include <engine/Engine.h>
#include <engine/Timer.h>


using namespace ad;

int main(int argc, const char * argv[])
{
    try
    {
        Application application("Spriting", 800, 600);

        ad::Timer timer{glfwGetTime(), 0.};

        Scene scene{ {400, 300} };

        while(application.nextFrame())
        {
            application.getEngine()->clear();
            timer.mark(glfwGetTime());
            scene.update(timer.time());
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
