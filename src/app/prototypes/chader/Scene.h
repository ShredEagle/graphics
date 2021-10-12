#pragma once

#include "Chader.h"
#include "FileWatcher.h"

#include <boost/filesystem.hpp>

namespace ad {
namespace graphics {

struct Scene
{
    Scene(const char * argv[]);
    void step();

    boost::filesystem::path mVertexShader;
    boost::filesystem::path mFragmentShader;
    Chader mChader;
    FileWatcher mFileWatcher;
};

Scene::Scene(const char * argv[]) :
    mVertexShader(argv[1]),
    mFragmentShader(argv[2]),
    mChader(),
    mFileWatcher({mVertexShader, mFragmentShader})
{
    mChader.loadProgram(mVertexShader, mFragmentShader);
}

void Scene::step()
{
    if (mFileWatcher.check())
    {
        std::cerr << "Found changes" << std::endl;
        mChader.loadProgram(mVertexShader, mFragmentShader);
    }
    mChader.render();
}


} // namespace graphics
} // namespace ad
