#include "SceneParallax.h"
#include "SceneSimple.h"

#include <graphics/ApplicationGlfw.h>
#include <graphics/AppInterface.h>
#include <graphics/Timer.h>

#include <math/VectorUtilities.h>


using namespace ad;
using namespace ad::graphics;

/// Usage:
/// Press enter to reverse scroll direction.
/// Press spacebar to toggle between scenes Simple and Parallax.
int main(int argc, const char * argv[])
{
    try
    {
        constexpr Size2<int> gWindowSize{800, 600};
        ApplicationGlfw application("Tiling", gWindowSize);

        Timer timer{glfwGetTime(), 0.};

        // Show exactly one tile in height.
        auto virtualResolution = math::makeSizeFromHeight(gCellSize.height(),
                                                          getRatio<float>(gWindowSize));
        SceneSimple simple{virtualResolution};
        SceneParallax parallax{virtualResolution};

        bool simpleScene = false;
        bool reverseScroll = false;
        application.getAppInterface()->registerKeyCallback([&reverseScroll, &simpleScene](int key, int, int action, int)
            {
                if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
                {
                    reverseScroll = !reverseScroll;
                }
                if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
                {
                    simpleScene = !simpleScene;
                }
            });

        double localTime = 0.;
        while(application.nextFrame())
        {
            application.getAppInterface()->clear();
            timer.mark(glfwGetTime());
            localTime += reverseScroll ? -timer.delta() : timer.delta();

            if (simpleScene)
            {
                simple.update(localTime);
                simple.render();
            }
            else
            {
                parallax.update(localTime);
                parallax.render();
            }
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
