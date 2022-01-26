#include "ParallaxScroller.h"

#include "../CameraUtilities.h"

#include <math/Transformations.h>


namespace ad {
namespace graphics {

//
// ParallaxScroller::Layer
//

ParallaxScroller::Layer::Layer(sprite::LoadedAtlas aAtlas,
                               Size2<int> aCellSize, Size2<int> aGridDefinition,
                               FillCallback aFillCallback,
                               float aScrollFactor) :
    atlas{std::move(aAtlas)},
    tileSet{aCellSize, aGridDefinition},
    fillCallback{std::move(aFillCallback)},
    placedTiles{tileSet.getTileCount(), TileSet::gEmptyInstance},
    scrollFactor{aScrollFactor}
{
    fillAll();
}


void ParallaxScroller::Layer::fillAll()
{
    auto gridDefinition = tileSet.getGridDefinition();
    for (int i = 0; i != gridDefinition.height(); ++i)
    {
        for (int j = 0; j != gridDefinition.width(); ++j)
        {
            placedTiles.at(j + i * gridDefinition.width()) = fillCallback({j, i});
        }
    }
    updateInstances();
}


void ParallaxScroller::Layer::updateInstances()
{
    tileSet.updateInstances(placedTiles);
}


Position2<TileSet::Position_t>
ParallaxScroller::Layer::getModulus(Position2<TileSet::Position_t> aPosition)
{
    return {
        (TileSet::Position_t)std::fmod(aPosition.x(), tileSet.getTileSize().width()) - tileSet.getTileSize().width(),
        (TileSet::Position_t)std::fmod(aPosition.y(), tileSet.getTileSize().height()) - tileSet.getTileSize().height()
    };
}


//
// ParallaxScroller
//
ParallaxScroller::ParallaxScroller(Size2<int> aVirtualResolution) :
    mViewportWorldSize{aVirtualResolution}
{
    setViewportVirtualResolution(mTiling, aVirtualResolution, ViewOrigin::LowerLeft);
}


void ParallaxScroller::addLayer(sprite::LoadedAtlas aAtlas, 
                                Size2<int> aCellSize,
                                Layer::FillCallback aFillCallback,
                                float aScrollFactor)
{
    mLayers.emplace_back(std::move(aAtlas), aCellSize, computeTightGrid(aCellSize), std::move(aFillCallback), aScrollFactor);
    // TODO Ad 2021/01/13: should save the latest camera position to set the position of the new layer 
    // (in case layers are dynamically added)
}


void ParallaxScroller::positionCamera(Position2<GLfloat> aPosition)
{
    for (auto & layer : mLayers)
    {
        auto newLayerPosition = -aPosition * layer.scrollFactor;
        layer.tileSet.setPosition(layer.getModulus(newLayerPosition));
    }
}


Size2<int> ParallaxScroller::computeTightGrid(Size2<int> aCellSize) const
{
    // +3 : 
    // * 1 for rounding up the division (the last tile, partially shown)
    // * 1 excess tile, i.e. initially completely "out of viewport"
    // * 1 for the offset applied to fmod
    return mViewportWorldSize.cwDiv(aCellSize) + Size2<GLint>{3, 3};
}


void ParallaxScroller::render() const
{
    for (const auto & layer : mLayers)
    {
        mTiling.render(layer.atlas, layer.tileSet);
    }
}

} // namespace graphics
} // namespace ad
