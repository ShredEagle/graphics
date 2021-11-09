#include "Scene.h"

#include <graphics/ApplicationGlfw.h>
#include <graphics/AppInterface.h>
#include <graphics/Timer.h>

#include <resource/PathProvider.h>

#include <future>
#include <thread>


using namespace ad;
using namespace ad::graphics;


std::string readLine()
{
    std::cout << "Your message: " << std::flush;
    std::string message;
    std::getline(std::cin, message);
    return message;
}


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

        std::future line = std::async(&readLine);
        while(application.nextFrame())
        {
            if (line.wait_for(std::chrono::seconds{0}) == std::future_status::ready)
            {
                scene.setMessage(line.get());
                line = std::async(&readLine);
            }
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
