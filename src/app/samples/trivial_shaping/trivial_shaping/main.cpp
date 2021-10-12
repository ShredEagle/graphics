#include "Scene.h"

#include <graphics/ApplicationGlfw.h>
#include <graphics/AppInterface.h>
#include <graphics/Timer.h>


using namespace ad;

int main(int argc, const char * argv[])
{
    try
    {
        ApplicationGlfw application("Trivial Shaping", 800, 600);

        ad::Timer timer{glfwGetTime(), 0.};

        Scene scene{ {400, 300} };

        while(application.nextFrame())
        {
            timer.mark(glfwGetTime());
            scene.step(timer);
            application.getAppInterface()->clear();
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
