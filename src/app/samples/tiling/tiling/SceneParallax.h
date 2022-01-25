#pragma once

#include <graphics/SpriteLoading.h>

#include <graphics/adapters/ParallaxScroller.h>

#include <resource/PathProvider.h>


namespace ad {
namespace graphics {


void addLayer(ParallaxScroller & aScroller, filesystem::path aImage, float aScrollFactor);


class SceneParallax
{
public:
    static constexpr Size2<int> gCellSize{320, 180}; // The images resolution
    static constexpr double gScrollSpeed = 150.;

    SceneParallax(Size2<int> aVirtualResolution) :
        mScroller{aVirtualResolution}
    {
        addLayer(mScroller, 
                 resource::pathFor("parallax/darkforest/DarkForest_Background.png"),
                 1.f/4.f);
        addLayer(mScroller, 
                 resource::pathFor("parallax/darkforest/DarkForest_Middleground.png"),
                 1.f/2.f);
        addLayer(mScroller, 
                 resource::pathFor("parallax/darkforest/DarkForest_Foreground.png"),
                 1.f);
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
};


void addLayer(ParallaxScroller & aScroller, filesystem::path aImage, float aScrollFactor)
{
    auto [atlas, loadedTiles] = 
        sprite::load(arte::Image<math::sdr::Rgba>{
                aImage,
                arte::ImageOrientation::InvertVerticalAxis});

    aScroller.addLayer(
        atlas, 
        SceneParallax::gCellSize,
        [tiles = std::move(loadedTiles)](Position2<int>){return tiles.at(0);},
        aScrollFactor);
}


} // namespace graphics
} // namespace ad
