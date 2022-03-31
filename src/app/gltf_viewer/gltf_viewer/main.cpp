#include "Logging.h"

#include "GltfRendering.h"
#include "Scene.h"

#include <arte/gltf/Gltf.h>
#include <arte/Logging.h>

#include <graphics/ApplicationGlfw.h>
#include <graphics/AppInterface.h>
#include <graphics/Timer.h>

#include <boost/program_options.hpp>
#include <boost/range/iterator_range.hpp>


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
    //spdlog::get(gltfviewer::gDrawLogger)->set_level(spdlog::level::trace);
}


filesystem::path pickFile(filesystem::path aUserPath)
{
    if(is_directory(aUserPath)) 
    {
        ADLOG(gltfviewer::gPrepareLogger, info)
         ("User path '{}' is a folder, looking for the first gltf file.", aUserPath);

        using boost::filesystem::directory_iterator;
        for(auto & entry : boost::make_iterator_range(directory_iterator(aUserPath), {}))
        {
            if(extension(entry) == ".gltf")
            {
                ADLOG(gltfviewer::gPrepareLogger, debug)("Picking file '{}'.", entry);
                return entry;
            }
        }
    }
    else if(is_regular_file(aUserPath))
    {
        return aUserPath;
    }
    ADLOG(gltfviewer::gPrepareLogger, critical)
         ("User path '{}' could not be associated to a gltf file.", aUserPath);
    throw std::runtime_error{"No input file."};
}



int main(int argc, const char * argv[])
{
    try
    {
        ::initializeLogging();

        po::variables_map arguments = handleCommandLineArguments(argc, argv);

        arte::Gltf gltf{pickFile(arguments["gltf-path"].as<std::string>())};

        arte::gltf::Index<arte::gltf::Scene> gltfSceneIndex = [&]()
        {
            if (auto defaultScene = gltf.getDefaultScene())
            {
                auto scene = *defaultScene;
                ADLOG(gltfviewer::gPrepareLogger, info)("Rendering default scene: {}.", *scene);
                return scene.id();
            }
            throw std::logic_error{"Viewer expects a default scene"};
        }();

        constexpr Size2<int> gWindowSize{1280, 1024};
        ApplicationGlfw application("glTF Viewer", gWindowSize);

        // Requires OpenGL context to call gl functions
        Scene viewerScene{gltf, gltfSceneIndex, application.getAppInterface()};

        Timer timer{glfwGetTime(), 0.};

        while(application.nextFrame())
        {
            application.getAppInterface()->clear();

            viewerScene.update(timer);
            viewerScene.render();

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
