#pragma once

#include <resource/PathProvider.h>

#include <2d/Engine.h>
#include <2d/Tiling.h>

#include <handy/random.h>

namespace ad
{

struct Scene {
    std::vector<Sprite> mSprites;
    Tiling mBackground;
};

inline Scene setupScene(Engine & aEngine)
{
    Scene result{
        aEngine.loadSheet(pathFor("tiles.bmp.meta")),
        Tiling{{32, 32}, {8, 5}}
    };

    aEngine.appendDraw(result.mSprites.at(0), {10, 10});

    Randomizer<std::uniform_int_distribution<unsigned char>> randomInt(0, 255);
    std::generate(result.mBackground.begin(),
                  result.mBackground.end(),
                  [&randomInt](){
                      return Color{randomInt(), randomInt(), randomInt()};
                  });

    return result;
}

inline void updateScene(Scene & aScene, Engine & aEngine, double aTimeSeconds)
{}

inline void renderScene(const Scene & aScene, Engine & aEngine)
{
    aEngine.clear();
    aScene.mBackground.render(aEngine);
    aEngine.render();
}

} // namespace ad
