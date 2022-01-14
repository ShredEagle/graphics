#pragma once


#include "../commons.h"

#include "../Tiling.h"

#include "../SpriteLoading.h"

#include <functional>


namespace ad {
namespace graphics {


// !!!
// ATTENTION 
// TODO Ad 2022/01/13: Call fill callback in response to camera position changes.
// !!!

class ParallaxScroller
{
    struct Layer
    {
        using FillCallback = std::function<LoadedSprite(Position2<int> /*aTileIndex*/)>;

        Layer(sprites::LoadedAtlas aAtlas,
              Size2<int> aCellSize, Size2<int> aGridDefinition,
              FillCallback aFillCallback,
              float aScrollFactor);

        void fillAll();
        void updateInstances();

        Position2<TileSet::Position_t> getModulus(Position2<TileSet::Position_t> aPosition);

        sprites::LoadedAtlas atlas;
        TileSet tileSet;
        FillCallback fillCallback;
        std::vector<TileSet::Instance> placedTiles;
        float scrollFactor;
    };

public:
    ParallaxScroller(Size2<int> aVirtualResolution);

    void addLayer(sprites::LoadedAtlas aAtlas, 
                  Size2<int> aCellSize,
                  Layer::FillCallback aFillCallback,
                  float aScrollFactor = 1.f);

    void positionCamera(Position2<GLfloat> aPosition);

    void render() const;

private:
    Size2<int> computeTightGrid(Size2<int> aCellSize) const;

    Size2<int> mViewportWorldSize;
    Tiling mTiling;
    std::vector<Layer> mLayers;
};


} // namespace graphics
} // namespace ad
