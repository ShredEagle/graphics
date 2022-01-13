#include "SceneParallax.h"
#include "SceneSimple.h"

#include <graphics/ApplicationGlfw.h>
#include <graphics/AppInterface.h>
#include <graphics/Timer.h>

#include <math/VectorUtilities.h>


using namespace ad;
using namespace ad::graphics;

/// Usage:
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
        application.getAppInterface()->registerKeyCallback([&simpleScene](int key, int, int action, int)
            {
                if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
                {
                    simpleScene = !simpleScene;
                }
            });

        while(application.nextFrame())
        {
            application.getAppInterface()->clear();
            timer.mark(glfwGetTime());
            if (simpleScene)
            {
                simple.update(timer.time());
                simple.render();
            }
            else
            {
                parallax.update(timer.time());
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
