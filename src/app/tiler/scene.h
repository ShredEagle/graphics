#pragma once

#include <resource/PathProvider.h>

#include <2d/Engine.h>

namespace ad
{

typedef std::vector<Sprite> Scene;

inline Scene setupScene(Engine & aEngine)
{
    Scene result = aEngine.loadSheet(pathFor("tiles.bmp.meta")); 
    aEngine.appendDraw(result.at(0), {10, 10});
    return result;
}

inline void updateScene(Scene & aScene, Engine & aEngine, double aTimeSeconds)
{}

} // namespace ad
