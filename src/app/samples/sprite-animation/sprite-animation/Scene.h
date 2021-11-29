#pragma once

#include <arte/SpriteSheet.h>

#include <graphics/Spriting.h>

#include <math/Interpolation/Interpolation.h>

#include <resource/PathProvider.h>

#include <optional>


namespace ad {
namespace graphics {


const std::string gAnimationName = "run";
// Animation is given in milliseconds, so natural speeds must be in the order of 10^3
const double gAnimationSpeed = 500;


class Animator
{
public:
    using Duration_t = arte::AnimationSpriteSheet::Duration_t;
    struct Frame
    {
        Duration_t endTime; // the local end time for this frame
        LoadedSprite loadedSprite;
    };

    struct Animation
    {
        Duration_t totalDuration;
        std::vector<Frame> frames;

        LoadedSprite at(Duration_t aLocalTime) const
        {
            auto frameIt = frames.begin();
            for (; frameIt != (frames.end() - 1); ++frameIt)
            {
                if (frameIt->endTime >= aLocalTime) break;
            }
            return frameIt->loadedSprite;
        }
    };

    template <class T_iterator>
    void load(T_iterator aSheetBegin, T_iterator aSheetEnd, Spriting & aSpriting)
    {
        // TODO
    }

    void load(const arte::AnimationSpriteSheet & aSpriteSheet, Spriting & aSpriting)
    {
        // Should not load empty sprite sheets.
        assert(aSpriteSheet.cbegin() != aSpriteSheet.cend());

        std::vector<Frame> animationFrames;
        Duration_t durationAccumulator = 0;
        aSpriting.loadCallback(aSpriteSheet.cbegin(), aSpriteSheet.cend(), aSpriteSheet.image(),
            [&](const LoadedSprite & aLoaded, const arte::AnimationSpriteSheet::Frame & aSourceFrame)
            {
                durationAccumulator += aSourceFrame.duration;
                animationFrames.push_back(Frame{
                    // Frame name is not saved at the moment
                    //aSourceFrame.name,
                    durationAccumulator,
                    aLoaded
                });
            }
        ); 
        mAnimations.emplace(aSpriteSheet.name(), 
                            Animation{aSpriteSheet.totalDuration(), std::move(animationFrames)});
    }

    const Animation & get(const std::string & aAnimationId) const
    {
        return mAnimations.at(aAnimationId);
    }

    LoadedSprite at(const std::string & aAnimationId, Duration_t aAnimationTime) const
    {
        return get(aAnimationId).at(aAnimationTime);
    }

    std::map<std::string, Animation> mAnimations;
};


class Scene
{
public:
    Scene(Size2<int> aRenderResolution) :
        mSpriting{std::move(aRenderResolution)}
    {
        arte::AnimationSpriteSheet sheet = 
            arte::AnimationSpriteSheet::LoadAseFile(resource::pathFor("animations/" + gAnimationName + ".json"));

        mAnimator.load(sheet, mSpriting);

        mAnimationParameter =
            ParameterAnimation_t{
                mAnimator.get(gAnimationName).totalDuration,
                gAnimationSpeed
        };
    }

    void update(double aDelta)
    {
        Spriting::Instance instance = Spriting::Instance{
            {0, 0},
            mAnimator.at(gAnimationName, mAnimationParameter->advance(aDelta)),
        };
        mSpriting.updateInstances(std::vector<Spriting::Instance>{instance});
    }

    void render()
    {
        mSpriting.render();
    }

private:

    // TODO hardcoded type here is not ideal
    using ParameterAnimation_t =
        //math::ParameterAnimation<double, math::FullRange>;
        math::ParameterAnimation<double, math::FullRange, math::periodic::Repeat>;
        //math::ParameterAnimation<double, math::FullRange, math::periodic::PingPong>;
        //math::ParameterAnimation<double, math::FullRange, math::periodic::Repeat, math::ease::SmoothStep>;
        //math::ParameterAnimation<double, math::FullRange, math::periodic::PingPong, math::ease::SmoothStep>;
        //math::ParameterAnimation<double, math::FullRange, math::None, math::ease::SmoothStep>;

    Spriting mSpriting;
    Animator mAnimator;
    std::optional<ParameterAnimation_t> mAnimationParameter;
};


} // namespace graphics
} // namespace ad
