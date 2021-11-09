#include "Scene.h"

#include <graphics/ApplicationGlfw.h>
#include <graphics/AppInterface.h>
#include <graphics/Timer.h>

#include <resource/PathProvider.h>


using namespace ad;
using namespace ad::graphics;

int main(int argc, const char * argv[])
{
    try
    {
        ApplicationGlfw application("Texting", 1600, 1000,
                                    ApplicationGlfw::Flags::None,
                                    4, 1,
                                    { {GLFW_SAMPLES, 8} });

        Timer timer{glfwGetTime(), 0.};
        Scene scene{resource::pathFor("fonts/dejavu-fonts-ttf-2.37/DejaVuSans.ttf"),
                    application.getAppInterface() };

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
