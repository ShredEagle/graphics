#pragma once


#include <resource/PathProvider.h>

#include <graphics/AppInterface.h>
#include <graphics/Spriting.h>
#include <graphics/Tiling.h>
#include <graphics/Timer.h>

#include <handy/random.h>

#include <math/Transformations.h>

#include <boost/iterator/iterator_adaptor.hpp>

#include <fstream>


namespace ad {
namespace graphics {

// Important: This adaptor is not required anymore since arte::TileSheet::Frame 
// is implicitly convertible to a const SpriteArea &.
// Keep the code around as an example regarding iterator_adaptor usage.
class SpriteArea_const_iter : public boost::iterator_adaptor<SpriteArea_const_iter,
                                                             arte::TileSheet::const_iterator,
                                                             const SpriteArea>
{
    // Inherit ctor
    using iterator_adaptor_::iterator_adaptor;

private:
    friend class boost::iterator_core_access;
    typename iterator_adaptor::reference dereference() const
    {
        return base_reference()->area;
    }
};

template <class T>
std::vector<LoadedSprite> loadSheet(T & aDrawer, const std::string & aFile)
{
    arte::TileSheet sheet = arte::TileSheet::LoadMetaFile(aFile);
    return aDrawer.load(SpriteArea_const_iter{sheet.cbegin()},
                        SpriteArea_const_iter{sheet.cend()},
                        sheet.image());
}

struct Scroller
{
    // Non-copiable, nor movable, because it captures `this` in a lambda registered externally
    Scroller(const Scroller &) = delete;
    Scroller & operator=(const Scroller &) = delete;

    Scroller(const Size2<int> aTileSize, std::string aTilesheet, AppInterface & aAppInterface) :
            mTiling(aTileSize,
                    aAppInterface.getWindowSize().cwDiv(aTileSize) + Size2<int>{2, 2},
                    aAppInterface.getWindowSize()),
            mTiles(loadSheet(mTiling, aTilesheet)),
            mRandomIndex(0, static_cast<int>(mTiles.size()-1))
    {
        fillRandom(mTiling.begin(), mTiling.end());

        mSizeListener = aAppInterface.listenFramebufferResize([this](Size2<int> aNewSize)
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

    void scroll(Vec2<GLfloat> aDisplacement, const AppInterface & aAppInterface)
    {
        mTiling.setPosition(mTiling.getPosition() + aDisplacement);

        Rectangle<GLfloat> grid(mTiling.getGridRectangle());
        GLint xDiff = static_cast<GLint>(grid.topRight().x())
                      - aAppInterface.getWindowSize().width();

        if (xDiff < 0)
        {
            reposition();
        }
    }

    void render(const AppInterface & aAppInterface) const
    {
        mTiling.render(aAppInterface);
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
    std::vector<LoadedSprite> mTiles; // The list of available tiles
    Randomizer<> mRandomIndex;
    std::shared_ptr<AppInterface::SizeListener> mSizeListener;
};


// For the static tile shown as a logo in the bottom left corner
struct Tiles
{
    // Non-copiable, nor movable, because it captures `this` in a lambda registered externally
    Tiles(const Tiles &) = delete;
    Tiles & operator=(const Tiles &) = delete;

    Tiles(std::string aSpriteSheet, AppInterface & aAppInterface)
    {
        mSpriting.setViewportVirtualResolution(aAppInterface.getWindowSize());
        mSpriting.setCameraTransformation(
            math::trans2d::translate(-static_cast<math::Vec<2, GLfloat>>(aAppInterface.getWindowSize()) / 2) );

        std::vector<LoadedSprite> frames{loadSheet(mSpriting, aSpriteSheet)};
        std::vector<Spriting::Instance> spriteInstances{
            Spriting::Instance{{20.f, 10.f}, frames.front()}};
        mSpriting.updateInstances(spriteInstances);

        mSizeListener = aAppInterface.listenFramebufferResize([this](Size2<int> aNewSize)
        {
            mSpriting.setViewportVirtualResolution(aNewSize);
            mSpriting.setCameraTransformation(
                math::trans2d::translate(-static_cast<math::Vec<2, GLfloat>>(aNewSize) / 2) );
        });
    }

    void render() const
    {
        mSpriting.render();
    }

private:
    Spriting mSpriting;
    std::shared_ptr<AppInterface::SizeListener> mSizeListener;
};


struct Scene
{
    Tiles mTiles;
    Scroller mBackground;
};



/// \note Scene is not copiable nor movable, hence return by pointer
inline std::unique_ptr<Scene> setupScene(AppInterface & aAppInterface)
{
    const Size2<int> tileSize{32, 32};
    std::unique_ptr<Scene> result(new Scene{
        Tiles{resource::pathFor("tiles.bmp.meta").string(), aAppInterface},
        Scroller{tileSize, resource::pathFor("tiles.bmp.meta").string(), aAppInterface}
    });

    /*
     * Sprites
     */

    return result;
}


inline void updateScene(Scene & aScene, AppInterface & aAppInterface, const Timer & aTimer)
{
    static const Vec2<GLfloat> scrollSpeed{-200.f, 0.f};
    aScene.mBackground.scroll((GLfloat)aTimer.mDelta*scrollSpeed, aAppInterface);
}


inline void renderScene(const Scene & aScene, AppInterface & aAppInterface)
{
    aAppInterface.clear();
    aScene.mBackground.render(aAppInterface);
    aScene.mTiles.render();
}


} // namespace graphics
} // namespace ad
