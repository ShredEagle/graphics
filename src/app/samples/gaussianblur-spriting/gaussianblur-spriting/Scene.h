#pragma once


#include <graphics/AppInterface.h>
#include <graphics/Spriting.h>
#include <graphics/effects/GaussianBlur.h>

#include <resource/PathProvider.h>

// Dirty include, to get the GLFW input definitions
#include <GLFW/glfw3.h>

#include <functional>


namespace ad {
namespace graphics {


constexpr int gBlurringPasses = 5;


class Scene
{
public:
    Scene(Size2<int> aRenderResolution, std::shared_ptr<AppInterface> aAppInterface) :
        mFrameBuffers{aRenderResolution},
        mSpriting{aRenderResolution}
    {
        using namespace std::placeholders;

        initializeSprite(aRenderResolution);
        aAppInterface->registerKeyCallback(std::bind(&Scene::onKey, this, _1, _2, _3, _4));
    }

    void update(double aDeltaSeconds)
    {
        constexpr double rotationsPerSec = 1.;
        const std::size_t frameCount = mSprites.size();

        if (mRotate)
        {
            mRotationFraction += aDeltaSeconds* rotationsPerSec;
        }

        mSpriteInstances.clear();
        mSpriteInstances.emplace_back(
            mPosition, 
            mSprites.at(static_cast<std::size_t>(mRotationFraction * frameCount) % frameCount)
        );
    }

    void render()
    {
        {
            // Render the scene to a texture
            auto renderTexture = mFrameBuffers.bindTargetFrameBuffer();
            auto viewport = mFrameBuffers.scopeViewport();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            mSpriting.render(mSpriteInstances);
            mFrameBuffers.swap();
        }
        if (mBlurring)
        {
            mBlur.apply(gBlurringPasses, mFrameBuffers, &FrameBuffer::Default());
        }
        else
        {
            // When blurring is not applied, we still have to draw to the default framebuffer.
            mBlur.drawToBoundFrameBuffer(mFrameBuffers);
        }
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

        const arte::ImageRgba ring{
            resource::pathFor("sonic_big_ring_1991_sprite_sheet_by_augustohirakodias_dc3iwce.png").string(),
            arte::ImageOrientation::InvertVerticalAxis
        };

        mSprites = mSpriting.load(frames.begin(), frames.end(), ring);
        mPosition = Position2<GLint>{(aRenderResolution - frameDimensions) / 2}; // centered
    }

    void onKey(int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
        {
            mRotate = !mRotate;
        }
        else if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS)
        {
            mRotationFraction = 0.;
        }
        else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        {
            mBlurring = !mBlurring;
        }
    }

    PingPongFrameBuffers mFrameBuffers;
    GaussianBlur mBlur;
    Spriting mSpriting;
    std::vector<LoadedSprite> mSprites;
    Position2<GLint> mPosition{0, 0};
    std::vector<Spriting::Instance> mSpriteInstances;

    // Controls
    bool mBlurring{true};
    bool mRotate{true};
    double mRotationFraction{0.};
};


} // namespace graphics
} // namespace ad
