#include "Scene.h"

#include <graphics/ApplicationGlfw.h>
#include <graphics/AppInterface.h>
#include <graphics/Timer.h>

#include <math/VectorUtilities.h>


using namespace ad;
using namespace ad::graphics;

/// Usage:
/// Press enter to reverse scroll direction.
int main(int argc, const char * argv[])
{
    try
    {
        constexpr Size2<int> gWindowSize{800, 600};
        ApplicationGlfw application("Tiling Callback", gWindowSize);

        Timer timer{glfwGetTime(), 0.};

        // Show 1 tile pixel per framebuffer pixel
        Scene scene{gWindowSize, *application.getAppInterface()};

        bool reverseScroll = false;
        application.getAppInterface()->registerKeyCallback([&reverseScroll, &scene](int key, int, int action, int)
            {
                if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
                {
                    reverseScroll = !reverseScroll;
                }
            });

        double localTime = 0.;
        while(application.nextFrame())
        {
            application.getAppInterface()->clear();
            timer.mark(glfwGetTime());
            localTime += reverseScroll ? -timer.delta() : timer.delta();

            scene.update(localTime);
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
