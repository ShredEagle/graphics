#pragma once

#include <graphics/CameraUtilities.h>
#include <graphics/SpriteLoading.h>
#include <graphics/Tiling.h>

#include <resource/PathProvider.h>


namespace ad {
namespace graphics {


constexpr Size2<int> gCellSize{320, 180}; // The images resolution
constexpr Size2<int> gGrid{2, 1};
constexpr double gScrollSpeed = 50.;

class Scene
{
public:
    Scene(Size2<int> aRenderResolution) :
        mTiling{gCellSize, gGrid}
    {
        setViewportVirtualResolution(mTiling, aRenderResolution, ViewOrigin::LowerLeft);

        sprites::LoadedAtlas atlas;
        std::tie(atlas, mLoadedTiles) = 
            sprites::load(arte::Image<math::sdr::Rgba>{
                    resource::pathFor("parallax/darkforest/DarkForest_Foreground.png"),
                    arte::ImageOrientation::InvertVerticalAxis});
        mTiling.load(atlas);

        std::ranges::fill(mPlacedTiles, mLoadedTiles.at(0));
        mTiling.updateInstances(mPlacedTiles);
    }

    void update(double aTimePointSeconds)
    {
        mTiling.setPosition({(float)(-aTimePointSeconds * gScrollSpeed), 0.f});
    }

    void render()
    {
        mTiling.render();
    }

private:
    Tiling mTiling;
    std::vector<LoadedSprite> mLoadedTiles; // The list of available tiles
    std::vector<Tiling::Instance> mPlacedTiles{gGrid.area(), Tiling::gEmptyInstance};
};


} // namespace graphics
} // namespace ad
