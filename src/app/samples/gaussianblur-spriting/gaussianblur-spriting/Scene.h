#pragma once

#include <graphics/Spriting.h>
#include <graphics/effects/GaussianBlur.h>

#include <resource/PathProvider.h>


namespace ad {
namespace graphics {


class Scene
{
public:
    Scene(Size2<int> aRenderResolution) :
        mFrameBuffers{aRenderResolution},
        mSpriting{aRenderResolution}
    {
        initializeSprite(aRenderResolution);
    }

    void update(double aTimeSeconds)
    {
        constexpr double rotationsPerSec = 1.5;
        constexpr double opacityCyclesPerSec = 0.5;
        constexpr double twoPi = 3.14159265359;
        const std::size_t frameCount = mSprites.size();

        mSpriteInstances.clear();
        mSpriteInstances.emplace_back(
            mPosition, 
            mSprites.at(static_cast<std::size_t>(aTimeSeconds*rotationsPerSec*frameCount) % frameCount),
            std::abs(std::cos(aTimeSeconds * opacityCyclesPerSec * twoPi))
        );
    }

    void render()
    {
        {
            // Render the scene to a texture
            auto renderTexture = mFrameBuffers.bindTargetFrameBuffer();
            auto viewport = mFrameBuffers.setupViewport();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            mSpriting.render(mSpriteInstances);
            mFrameBuffers.swap();
        }
        mBlur.drawToBoundFrameBuffer(mFrameBuffers);
    }

private:
    void initializeSprite(Size2<int> aRenderResolution)
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

        const Image ring(pathFor("sonic_big_ring_1991_sprite_sheet_by_augustohirakodias_dc3iwce.png").string());
        mSprites = mSpriting.load(frames.begin(), frames.end(), ring);
        mPosition = Position2<GLint>{(aRenderResolution - frameDimensions) / 2}; // centered
    }

    PingPongFrameBuffers mFrameBuffers;
    GaussianBlur mBlur;
    Spriting mSpriting;
    std::vector<LoadedSprite> mSprites;
    Position2<GLint> mPosition{0, 0};
    std::vector<Spriting::Instance> mSpriteInstances;
};


} // namespace graphics
} // namespace ad
