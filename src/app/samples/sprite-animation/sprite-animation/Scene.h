#pragma once

#include <arte/SpriteSheet.h>

#include <graphics/Spriting.h>

#include <math/Interpolation.h>

#include <resource/PathProvider.h>

#include <optional>


namespace ad {
namespace graphics {


// TODO rename
namespace periodicity
{
    template <class T_parameter>
    class Periodic_base
    {
    public:
        Periodic_base(T_parameter aDuration) :
            mDuration{std::move(aDuration)}
        {}

    protected:
        T_parameter mDuration;
    };

    template <class T_parameter>
    class Repeat : Periodic_base<T_parameter>
    {
        using Periodic_base<T_parameter>::Periodic_base;

    public:
        // TODO find a name
        T_parameter operator()(T_parameter aAbsoluteValue) const
        {
            return aAbsoluteValue - std::floor(aAbsoluteValue/mDuration) * mDuration;
        }
    };

    template <class T_parameter>
    class PingPong : Periodic_base<T_parameter>
    {
        using Periodic_base<T_parameter>::Periodic_base;

    public:
        // TODO find a name
        T_parameter operator()(T_parameter aAbsoluteValue) const
        {
            T_parameter doubleDuration = 2 * mDuration;
            T_parameter doublePeriod =
                aAbsoluteValue - std::floor(aAbsoluteValue/doubleDuration) * doubleDuration;

            return doublePeriod - 2 * std::max(T_parameter{0}, doublePeriod - mDuration);
        }
    };
} // namespace periodicity

template <class T_parameter,
          class T_periodicity,
          class T_easeFunctor = math::ease::Linear<T_parameter>>

class UnboundParameterAnimation
{
public:
    using Result_type = T_parameter;

    
    explicit UnboundParameterAnimation(T_parameter aSpeedFactor, T_periodicity aBehaviour) :
        mBehaviour{std::move(aBehaviour)},
        mSpeedFactor{aSpeedFactor}
    {}


    Result_type at(T_parameter aInput) const
    {
        return T_easeFunctor::ease(mBehaviour(mSpeedFactor * aInput));
    }


    Result_type advance(T_parameter aIncrement)
    {
        return at(mAccumulatedInput += aIncrement);
    }


    // Not easily generalized
    //bool isCompleted() const
    //{
    //    return mAccumulatedInput * mSpeedFactor >= Result_type::high_v;
    //}


private:
    T_periodicity mBehaviour;
    T_parameter mSpeedFactor;
    T_parameter mAccumulatedInput{0};
};


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
            arte::AnimationSpriteSheet::LoadAseFile(resource::pathFor("animations/run.json"));

        mAnimator.load(sheet, mSpriting);

        // Animation is given in milliseconds, so multiply speed by 10^3
        mAnimationParameter =
            //UnboundParameterAnimation<double, periodicity::PingPong<double>>{
            UnboundParameterAnimation<double, periodicity::Repeat<double>>{
                1000,
                {mAnimator.get("run").totalDuration}
        };
    }

    void update(double aDelta)
    {
        Spriting::Instance instance = Spriting::Instance{
            {0, 0},
            mAnimator.at("run", mAnimationParameter->advance(aDelta)),
        };
        mSpriting.updateInstances(std::vector<Spriting::Instance>{instance});
    }

    void render()
    {
        mSpriting.render();
    }

private:
    Spriting mSpriting;
    Animator mAnimator;
    // TODO hardcoded type here is not ideal
    std::optional<UnboundParameterAnimation<double, periodicity::Repeat<double>>> mAnimationParameter;
    //std::optional<UnboundParameterAnimation<double, periodicity::PingPong<double>>> mAnimationParameter;
};


} // namespace graphics
} // namespace ad
