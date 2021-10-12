#include "Scene.h"

#include <graphics/ApplicationGlfw.h>
#include <graphics/AppInterface.h>
#include <graphics/Timer.h>


int main(int argc, const char * argv[])
{
    try
    {
        ad::ApplicationGlfw application("Polyline", 800, 600);

        ad::Timer timer{glfwGetTime(), 0.};

        ad::Scene scene;

        while(application.nextFrame())
        {
            application.getAppInterface()->clear();
            scene.step(timer);
            scene.render(application.getAppInterface()->getWindowSize());
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