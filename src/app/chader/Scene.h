#pragma once

#include "Chader.h"
#include "FileWatcher.h"

namespace ad {

struct Scene
{
    Scene(const char * argv[]);
    void step();

    path mVertexShader;
    path mFragmentShader;
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


} // namespace ad
