#include "Scene.h"

#include <engine/Application.h>
#include <engine/Engine.h>
#include <engine/Timer.h>


int main(int argc, const char * argv[])
{
    try
    {
        ad::Application application("_bootstrap", 800, 600);

        ad::Timer timer{glfwGetTime(), 0.};

        ad::Scene scene;

        while(application.nextFrame())
        {
            application.getEngine()->clear();
            scene.step(timer);
            scene.render();
            timer.mark(glfwGetTime());
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