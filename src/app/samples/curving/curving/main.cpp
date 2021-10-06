#include "Scene.h"

#include <engine/Application.h>
#include <engine/Engine.h>
#include <engine/Timer.h>


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
        Application application("Curving", 1600, 1000,
                                 Application::Flags::None,
                                 4, 1,
                                 { {GLFW_SAMPLES, 8} });

        ad::Timer timer{glfwGetTime(), 0.};

        Scene scene{application.getEngine()->getFramebufferSize(), application.getEngine() };

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
