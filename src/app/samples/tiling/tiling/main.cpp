#include "Scene.h"

#include <graphics/ApplicationGlfw.h>
#include <graphics/AppInterface.h>
#include <graphics/Timer.h>

#include <math/VectorUtilities.h>


using namespace ad;
using namespace ad::graphics;

int main(int argc, const char * argv[])
{
    try
    {
        constexpr Size2<int> gWindowSize{800, 600};
        ApplicationGlfw application("Tiling", gWindowSize);

        Timer timer{glfwGetTime(), 0.};

        // Show exactly one tile in height.
        Scene scene{ math::makeSizeFromHeight(gCellSize.height(),
                                              getRatio<float>(gWindowSize)) };

        while(application.nextFrame())
        {
            application.getAppInterface()->clear();
            timer.mark(glfwGetTime());
            scene.update(timer.time());
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
