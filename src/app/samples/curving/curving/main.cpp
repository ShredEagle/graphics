#include "Scene.h"

#include <graphics/ApplicationGlfw.h>
#include <graphics/AppInterface.h>
#include <graphics/Timer.h>


using namespace ad;

// Control the middle point out handle with left button drag.
// Press enter to alternate between rotation / side view.
// Press backspace to reset rotation to zero.
// Press B to troggle curve width beating.
// Press F to troggle wireframe rendering.

int main(int argc, const char * argv[])
{
    try
    {
        ApplicationGlfw application("Curving", 1600, 1000,
                                 ApplicationGlfw::Flags::None,
                                 4, 1,
                                 { {GLFW_SAMPLES, 8} });

        ad::Timer timer{glfwGetTime(), 0.};

        Scene scene{application.getAppInterface()->getFramebufferSize(), application.getAppInterface() };

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
