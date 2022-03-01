#pragma once


#include "../commons.h"

#include "../Tiling.h"

#include "../SpriteLoading.h"

#include <functional>


namespace ad {
namespace graphics {


class ParallaxScroller
{
    struct Layer
    {
        using FillCallback = std::function<LoadedSprite(Position2<int> /*aTileIndex*/)>;

        Layer(sprite::LoadedAtlas aAtlas,
              Size2<int> aCellSize, Size2<int> aGridDefinition,
              FillCallback aFillCallback,
              float aScrollFactor);

        void fillAll(math::Vec<2, int> aFirstTileIndex);
        void updateInstances();

        void setPosition(Position2<TileSet::Position_t> aPosition);

        void resetTiling(Size2<int> aGridDefinition);

        Position2<TileSet::Position_t> getModulus(Position2<TileSet::Position_t> aPosition);

        sprite::LoadedAtlas atlas;
        TileSet tileSet;
        math::Vec<2, int> firstTileIndex{-1, -1};
        FillCallback fillCallback;
        std::vector<TileSet::Instance> placedTiles;
        float scrollFactor;
    };

public:
    /// \param aVirtualResolution Virtual resolution of the viewport in term of cell pixels.
    /// (i.e., if the virtual resolution was the cell size, exactly one cell would fit the viewport).
    ParallaxScroller(Size2<int> aVirtualResolution);

    void addLayer(sprite::LoadedAtlas aAtlas, 
                  Size2<int> aCellSize,
                  Layer::FillCallback aFillCallback,
                  float aScrollFactor = 1.f);

    void resetTiling(Size2<int> aVirtualResolution);

    void positionCamera(Position2<GLfloat> aPosition);

    void render() const;

private:
    Size2<int> computeTightGrid(Size2<int> aCellSize) const;

    Size2<int> mViewportSize_cellPixels;
    Tiling mTiling;
    std::vector<Layer> mLayers;
};


} // namespace graphics
} // namespace ad
