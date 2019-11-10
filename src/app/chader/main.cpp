#include "Chader.h"

#include <engine/Application.h>
#include <engine/Engine.h>
#include <engine/Timer.h>


using namespace ad;

int main(int argc, char ** argv)
{
    Application application("Chader", 800, 600);

    //std::unique_ptr<ad::Scene> scene = ad::setupScene(engine);
    ad::Timer timer{glfwGetTime(), 0.};

    Chader chad;

    while(application.nextFrame())
    {
        //ad::updateScene(*scene, engine, timer);
        //ad::renderScene(*scene, engine);

        timer.mark(glfwGetTime());
    }

    std::exit(EXIT_SUCCESS);
}
