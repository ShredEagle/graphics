#pragma once

#include <arte/SpriteSheet.h>

#include <graphics/AppInterface.h>
#include <graphics/adapters/ParallaxScroller.h>

#include <test_commons/PathProvider.h>


namespace ad {
namespace graphics {


LoadedSprite fill(std::vector<LoadedSprite> aSprites, Position2<int> aTileIndex)
{
    return *(aSprites.begin() + (aTileIndex.x() % aSprites.size()));
}


class Scene
{
public:
    static constexpr double gScrollSpeed = 50.;

    Scene(Size2<int> aVirtualResolution, AppInterface & aAppInterface) :
        mScroller{aVirtualResolution},
        mTileSheet{arte::TileSheet::LoadMetaFile(resource::pathFor("tiles.bmp.meta"))}
    {
        sprite::LoadedAtlas atlas;
        std::tie(atlas, mLoadedSprites) = sprite::load(mTileSheet);

        mScroller.addLayer(
            std::move(atlas), 
            mTileSheet.cbegin()->area.dimension(), // all tiles are of the same size
            std::bind(fill, std::cref(mLoadedSprites), std::placeholders::_1),
            1.0f);

        mSizeListener = aAppInterface.listenFramebufferResize([this](Size2<int> aNewSize)
        {
            mScroller.resetTiling(aNewSize);
        });
    }

    void update(double aTimePointSeconds)
    {
        Position2<GLfloat> camera{
            (float)(aTimePointSeconds * gScrollSpeed),
            0.f
        };
        mScroller.positionCamera(camera);
    }

    void render()
    {
        mScroller.render();
    }

private:
    ParallaxScroller mScroller;
    arte::TileSheet mTileSheet;
    std::vector<LoadedSprite> mLoadedSprites;
    std::shared_ptr<AppInterface::SizeListener> mSizeListener;
};


} // namespace graphics
} // namespace ad
