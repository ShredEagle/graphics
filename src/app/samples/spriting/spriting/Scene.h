#pragma once

#include <graphics/Spriting.h>

#include <resource/PathProvider.h>


namespace ad {
namespace graphics {


class Scene
{
public:
    Scene(Size2<int> aRenderResolution) :
        mSpriting{std::move(aRenderResolution)}
    {
        constexpr Size2<int> frameDimensions{347-3, 303-3};

        // Complete ring animation
        std::initializer_list<Rectangle<int>> frames = {
            {{3,    3}, frameDimensions},
            {{353,  3}, frameDimensions},
            {{703,  3}, frameDimensions},
            {{1053, 3}, frameDimensions},
            {{1403, 3}, frameDimensions},
            {{1753, 3}, frameDimensions},
            {{2103, 3}, frameDimensions},
            {{2453, 3}, frameDimensions},
        };

        const arte::ImageRgba ring{
            resource::pathFor("sonic_big_ring_1991_sprite_sheet_by_augustohirakodias_dc3iwce.png").string(),
            arte::ImageOrientation::InvertVerticalAxis
        };

        mSprites = mSpriting.load(frames.begin(), frames.end(), ring);
        mPosition = Position2<GLint>{(aRenderResolution - frameDimensions) / 2}; // centered
    }

    void update(double aTimeSeconds)
    {
        constexpr double rotationsPerSec = 1.5;
        constexpr double opacityCyclesPerSec = 0.5;
        constexpr double twoPi = 3.14159265359;
        const std::size_t frameCount = mSprites.size();

        std::vector<Spriting::Instance> spriteInstances;
        spriteInstances.emplace_back(
            mPosition, 
            mSprites.at(static_cast<std::size_t>(aTimeSeconds*rotationsPerSec*frameCount) % frameCount),
            std::abs(std::cos(aTimeSeconds * opacityCyclesPerSec * twoPi))
        );
        mSpriting.updateInstances(spriteInstances);
    }

    void render()
    {
        mSpriting.render();
    }

private:
    Spriting mSpriting;
    std::vector<LoadedSprite> mSprites;
    Position2<GLint> mPosition{0, 0};
};


} // namespace graphics
} // namespace ad
