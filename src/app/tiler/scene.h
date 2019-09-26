#pragma once


#include <resource/PathProvider.h>

#include <2d/Engine.h>
#include <2d/Tiling.h>
#include <2d/dataformat/tiles.h>

#include <handy/random.h>

#include <boost/iterator/iterator_adaptor.hpp>

#include <fstream>


namespace ad
{

struct Scene {
    std::vector<Sprite> mSprites;
    Tiling mBackground;
};

class SpriteArea_const_iter : public boost::iterator_adaptor<SpriteArea_const_iter,
                                                             std::vector<Sprite>::const_iterator,
                                                             const SpriteArea>
{
    // Inherit ctor
    using SpriteArea_const_iter::iterator_adaptor_::iterator_adaptor;

private:
    friend class boost::iterator_core_access;
    typename iterator_adaptor::reference dereference() const
    {
        return base_reference()->mTextureArea;
    }
};

std::vector<LoadedSprite> loadTilesFromFile(Tiling & aTiling, const std::string &aFile)
{
    std::ifstream ifs{aFile};
    SpriteSheet sheet = dataformat::loadMeta(ifs);
    return aTiling.load(SpriteArea_const_iter{sheet.mSprites.cbegin()},
                        SpriteArea_const_iter{sheet.mSprites.cend()},
                        sheet.mRasterData);
}

inline Scene setupScene(Engine & aEngine)
{
    Scene result{
        aEngine.loadSheet(pathFor("tiles.bmp.meta")),
        Tiling{{32, 32}, {16, 16}}
    };

    /*
     * Sprites
     */
    aEngine.appendDraw(result.mSprites.at(0), {10, 10});

    /*
     * Tiles
     */
    std::vector<LoadedSprite> tiles =
        loadTilesFromFile(result.mBackground, pathFor("tiles.bmp.meta"));

    std::fill(result.mBackground.begin(), result.mBackground.end(), tiles.front());

    Randomizer<> randomIndex(0, tiles.size()-1);
    std::generate(result.mBackground.begin(),
                  result.mBackground.end(),
                  [&randomIndex, &tiles](){
                      return tiles.at(randomIndex());
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
