#include "Scene.h"

#include <graphics/ApplicationGlfw.h>
#include <graphics/AppInterface.h>
#include <graphics/Timer.h>


using namespace ad;
using namespace ad::graphics;

// Press enter to toggle rotation.
// Press backspace to reset rotation to zero.
// Press space to toggle blurring.

int main(int argc, const char * argv[])
{
    try
    {
        ApplicationGlfw application("Spriting", 800, 600);

        Timer timer{glfwGetTime(), 0.};

        Scene scene{ {400, 300}, application.getAppInterface() };

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
