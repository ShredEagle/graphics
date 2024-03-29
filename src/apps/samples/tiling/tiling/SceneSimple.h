#pragma once

#include <graphics/CameraUtilities.h>
#include <graphics/SpriteLoading.h>
#include <graphics/Tiling.h>

#include <test_commons/PathProvider.h>

#include <vector>


namespace ad {
namespace graphics {


constexpr Size2<int> gCellSize{320, 180}; // The images resolution
constexpr double gScrollSpeed = 150.;

class SceneSimple
{
public:
    SceneSimple(Size2<int> aVirtualResolution) :
        // +2 :
        // * 1 for rounding up the division (the last tile, partially shown)
        // * 1 excess tile, i.e. initially completely "out of viewport"
        mGridSize{(aVirtualResolution.width() / gCellSize.width()) + 2, 1},
        mTiling{},
        mTileSet{gCellSize, mGridSize}
    {
        setViewedSize(mTiling, aVirtualResolution, ViewOrigin::LowerLeft);

        auto [atlas, loadedTile] =
            sprite::load(arte::Image<math::sdr::Rgba>{
                    resource::pathFor("parallax/darkforest/DarkForest_Foreground.png"),
                    arte::ImageOrientation::InvertVerticalAxis});
        mAtlas = std::move(atlas);

        std::ranges::fill(mPlacedTiles, loadedTile);
        mTileSet.updateInstances(mPlacedTiles);
    }

    void update(double aTimePointSeconds)
    {
        Position2<GLfloat> gridPosition{
            (float)std::fmod(-aTimePointSeconds * gScrollSpeed, gCellSize.width()),
            0.f
        };

        if (gridPosition.x() > 0)
        {
            gridPosition.x() -= gCellSize.width();
        }
        mTileSet.setPosition(gridPosition);
    }

    void render()
    {
        mTiling.render(mAtlas, mTileSet);
    }

private:
    Size2<int> mGridSize;
    Tiling mTiling;
    TileSet mTileSet;
    sprite::LoadedAtlas mAtlas;
    std::vector<TileSet::Instance> mPlacedTiles{static_cast<size_t>(mGridSize.area()), TileSet::gEmptyInstance};
};


} // namespace graphics
} // namespace ad
