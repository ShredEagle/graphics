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

struct Timer
{
    void mark(double aMonotonic)
    {
        mDelta = aMonotonic - mTime;
        mTime = aMonotonic;
    }

    double mTime;
    double mDelta;
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

struct Scroller
{
    Scroller(const Size2<int> aTileSize, path aTilesheet, Engine & aEngine) :
            mTiling(aTileSize,
                    hadamardDiv(aEngine.getWindowSize(), aTileSize) + Size2<int>{1, 1},
                    aEngine.getWindowSize()),
            mTiles(loadTilesFromFile(mTiling, pathFor(aTilesheet))),
            mRandomIndex(0, mTiles.size()-1)
    {
        fillRandom(mTiling.begin(), mTiling.end());

        aEngine.listenResize([this](Size2<int> aNewSize)
        {
            mTiling.setBufferResolution(aNewSize);
            // +2 tiles on each dimension:
            // * 1 to compensate for the integral division module
            // * 1 to make sure there is at least the size of a complete tile in excess
            Size2<int> gridDefinition = hadamardDiv(aNewSize, mTiling.getTileSize()) + Size2<int>{2, 2};
            mTiling.resetTiling(mTiling.getTileSize(), gridDefinition);
            fillRandom(mTiling.begin(), mTiling.end());
        });
    }

    void scroll(Vec2<GLfloat> aDisplacement, const Engine & aEngine)
    {
        mTiling.setPosition(mTiling.getPosition() + aDisplacement);

        Rectangle<GLfloat> grid(mTiling.getGridRectangle());
        // Integral conversion
        GLint xDiff = grid.diagonalCorner().x() - aEngine.getWindowSize().width();

        if (xDiff < 0)
        {
            reposition();
        }
    }

    void render(const Engine & aEngine) const
    {
        mTiling.render(aEngine);
    }

private:
    void fillRandom(Tiling::iterator aFirst, Tiling::iterator aLast)
    {
        std::generate(aFirst,
                      aLast,
                      [this](){
                          return mTiles.at(mRandomIndex());
                      });
    }

    void reposition()
    {
        mTiling.setPosition(mTiling.getPosition()
                            + static_cast<Vec2<GLfloat>>(mTiling.getTileSize().hadamard({1, 0})));

        // Copy the tile still appearing
        std::copy(mTiling.begin()+mTiling.getGridDefinition().height(),
                  mTiling.end(),
                  mTiling.begin());

        // Complete new tiles
        fillRandom(mTiling.end()-mTiling.getGridDefinition().height(), mTiling.end());
    }

private:
    Tiling mTiling;
    std::vector<LoadedSprite> mTiles;
    Randomizer<> mRandomIndex;
};

struct Scene {
    std::vector<Sprite> mSprites;
    Scroller mBackground;
};



inline Scene setupScene(Engine & aEngine)
{
    const Size2<int> tileSize{32, 32};
    Scene result{
        aEngine.loadSheet(pathFor("tiles.bmp.meta")),
        Scroller{tileSize, "tiles.bmp.meta", aEngine}
    };

    /*
     * Sprites
     */
    aEngine.appendDraw(result.mSprites.at(0), {10, 10});

    /*
     * Tiles
     */

    return result;
}

inline void updateScene(Scene & aScene, Engine & aEngine, const Timer & aTimer)
{
    static const Vec2<GLfloat> scrollSpeed{-200.f, 0.f};
    aScene.mBackground.scroll((GLfloat)aTimer.mDelta*scrollSpeed, aEngine);
}

inline void renderScene(const Scene & aScene, Engine & aEngine)
{
    aEngine.clear();
    aScene.mBackground.render(aEngine);
    aEngine.render();
}

} // namespace ad
