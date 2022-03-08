#include "Logging.h"

#include "GltfRendering.h"

#include <arte/Gltf.h>
#include <arte/Logging.h>

#include <graphics/ApplicationGlfw.h>
#include <graphics/AppInterface.h>
#include <graphics/Timer.h>

#include <boost/program_options.hpp>


using namespace ad;
using namespace ad::graphics;
using namespace ad::gltfviewer;

namespace po = boost::program_options;


po::variables_map handleCommandLineArguments(int argc, const char ** argv)
{
    po::options_description desc("Gltf viewer.");
    desc.add_options()
        ("help", "Produce help message.")
        ("gltf-path", po::value<std::string>()->required(), "Path to a glTF file to be viewed.");
    ;

    po::positional_options_description positional;
    positional.add("gltf-path", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(positional).run(),
              vm);

    if (vm.count("help"))
    {
        std::cout << desc << "\n";
    }

    po::notify(vm);

    return vm;
}


void initializeLogging()
{
    arte::initializeLogging();
    gltfviewer::initializeLogging();
    spdlog::get(gltfviewer::gPrepareLogger)->set_level(spdlog::level::trace);
    spdlog::get(gltfviewer::gDrawLogger)->set_level(spdlog::level::debug);
}



int main(int argc, const char * argv[])
{
    try
    {
        ::initializeLogging();

        po::variables_map arguments = handleCommandLineArguments(argc, argv);

        arte::Gltf gltf{arguments["gltf-path"].as<std::string>()};

        constexpr Size2<int> gWindowSize{800, 600};
        ApplicationGlfw application("glTF Viewer", gWindowSize);

        // Requires OpenGL context to render
        arte::Const_Owned<arte::gltf::Scene> scene = [&]()
        {
            if (auto defaultScene = gltf.getDefaultScene())
            {
                auto scene = *defaultScene;
                ADLOG(gltfviewer::gPrepareLogger, info)("Rendering default scene: {}.", *scene);
                return scene;
            }
            throw std::logic_error{"Viewer expects a default scene"};
        }();

        std::vector<Mesh> meshes;
        for (arte::Const_Owned<arte::gltf::Node> node : scene.iterate(&arte::gltf::Scene::nodes))
        {
            meshes.push_back(prepare(node.get(&arte::gltf::Node::mesh)));
        }

        Timer timer{glfwGetTime(), 0.};

        while(application.nextFrame())
        {
            application.getAppInterface()->clear();

            for (const Mesh & mesh : meshes)
            {
                render(mesh);
            }

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
