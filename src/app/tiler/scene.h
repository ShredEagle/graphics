#pragma once


#include <resource/PathProvider.h>

#include <engine/Engine.h>
#include <engine/Spriting.h>
#include <engine/Tiling.h>
#include <engine/Timer.h>
#include <engine/dataformat/tiles.h>

#include <handy/random.h>

#include <boost/iterator/iterator_adaptor.hpp>

#include <fstream>


namespace ad
{

class SpriteArea_const_iter : public boost::iterator_adaptor<SpriteArea_const_iter,
                                                             std::vector<Sprite>::const_iterator,
                                                             const SpriteArea>
{
    // Inherit ctor
    using iterator_adaptor_::iterator_adaptor;

private:
    friend class boost::iterator_core_access;
    typename iterator_adaptor::reference dereference() const
    {
        return base_reference()->mTextureArea;
    }
};

template <class T>
std::vector<LoadedSprite> loadSheet(T & aDrawer, const std::string & aFile)
{
    std::ifstream ifs{aFile};
    SpriteSheet sheet = dataformat::loadMeta(ifs);
    return aDrawer.load(SpriteArea_const_iter{sheet.mSprites.cbegin()},
                        SpriteArea_const_iter{sheet.mSprites.cend()},
                        sheet.mRasterData);
}

struct Scroller
{
    // Non-copiable, nor movable, because it captures `this` in a lambda registered externally
    Scroller(const Scroller &) = delete;
    Scroller & operator=(const Scroller &) = delete;

    Scroller(const Size2<int> aTileSize, std::string aTilesheet, Engine & aEngine) :
            mTiling(aTileSize,
                    aEngine.getWindowSize().cwDiv(aTileSize) + Size2<int>{1, 1},
                    aEngine.getWindowSize()),
            mTiles(loadSheet(mTiling, aTilesheet)),
            mRandomIndex(0, static_cast<int>(mTiles.size()-1))
    {
        fillRandom(mTiling.begin(), mTiling.end());

        aEngine.listenResize([this](Size2<int> aNewSize)
        {
            mTiling.setBufferResolution(aNewSize);
            // +2 tiles on each dimension:
            // * 1 to compensate for the integral division module
            // * 1 to make sure there is at least the size of a complete tile in excess
            Size2<int> gridDefinition = aNewSize.cwDiv(mTiling.getTileSize()) + Size2<int>{2, 2};
            mTiling.resetTiling(mTiling.getTileSize(), gridDefinition);
            fillRandom(mTiling.begin(), mTiling.end());
        });
    }

    void scroll(Vec2<GLfloat> aDisplacement, const Engine & aEngine)
    {
        mTiling.setPosition(mTiling.getPosition() + aDisplacement);

        Rectangle<GLfloat> grid(mTiling.getGridRectangle());
        GLint xDiff = static_cast<GLint>(grid.topRight().x())
                      - aEngine.getWindowSize().width();

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
                            + static_cast<Vec2<GLfloat>>(mTiling.getTileSize().cwMul({1, 0})));

        // Copy the tiles still appearing
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

struct RingDrop
{
    // Non-copiable, nor movable, because it captures `this` in a lambda registered externally
    RingDrop(const RingDrop &) = delete;
    RingDrop & operator=(const RingDrop &) = delete;

    RingDrop(std::string aSpriteSheet, Engine & aEngine) :
             mSpriting(aEngine.getWindowSize()),
             mFrames(loadSheet(mSpriting, aSpriteSheet))
    {
        mSpriting.instanceData().push_back(Instance{{20, 10}, mFrames.front()});

        aEngine.listenResize([this](Size2<int> aNewSize)
        {
            mSpriting.setBufferResolution(aNewSize);
        });
    }

    void render() const
    {
        mSpriting.render();
    }

private:
    Spriting mSpriting;
    std::vector<LoadedSprite> mFrames;
};

struct Scene {
    RingDrop mRings;
    Scroller mBackground;
};



/// \note Scene is not copiable nor movable, hence return by pointer
inline std::unique_ptr<Scene> setupScene(Engine & aEngine)
{
    const Size2<int> tileSize{32, 32};
    std::unique_ptr<Scene> result(new Scene{
        RingDrop{pathFor("tiles.bmp.meta").string(), aEngine},
        Scroller{tileSize, pathFor("tiles.bmp.meta").string(), aEngine}
    });

    /*
     * Sprites
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
    aScene.mRings.render();
}

} // namespace ad
