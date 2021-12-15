#include "Scene.h"

#include <graphics/ApplicationGlfw.h>
#include <graphics/AppInterface.h>
#include <graphics/Timer.h>


using namespace ad;
using namespace ad::graphics;

int main(int argc, const char * argv[])
{
    try
    {
        ApplicationGlfw application("Sprite Animation", 800, 600);

        Timer timer{glfwGetTime(), 0.};

        Scene scene{ {200, 150} };

        while(application.nextFrame())
        {
            application.getAppInterface()->clear();
            timer.mark(glfwGetTime());
            scene.update(timer.delta());
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
