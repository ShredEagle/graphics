#include "Scene.h"

#include <engine/Application.h>
#include <engine/Engine.h>
#include <engine/Timer.h>


using namespace ad;

int main(int argc, const char * argv[])
{
    try
    {
        Application application("Bloom", 800, 600);

        ad::Timer timer{glfwGetTime(), 0.};

        Scene scene(argv, application.mEngine.get());

        while(application.nextFrame())
        {
            scene.step();
            timer.mark(glfwGetTime());
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
