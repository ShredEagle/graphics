#include "Scene.h"

#include <graphics/Application.h>
#include <graphics/Engine.h>
#include <graphics/Timer.h>


int main(int argc, const char * argv[])
{
    try
    {
        ad::Application application("Curve", 800, 600);

        ad::Timer timer{glfwGetTime(), 0.};

        ad::curve::Scene scene;

        while(application.nextFrame())
        {
            application.getEngine()->clear();
            scene.step(timer);
            scene.render(application.getEngine()->getWindowSize());
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
