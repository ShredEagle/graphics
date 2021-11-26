#pragma once

#include <arte/SpriteSheet.h>

#include <graphics/Spriting.h>

#include <math/Interpolation.h>

#include <resource/PathProvider.h>

#include <optional>


namespace ad {
namespace graphics {


const std::string gAnimationName = "number_test";


namespace periodicity
{
    template <class T_parameter>
    struct Repeat
    {
        // TODO give a name?
        T_parameter operator()(T_parameter aPeriod, T_parameter aAbsoluteValue) const
        {
            return aAbsoluteValue - std::floor(aAbsoluteValue/aPeriod) * aPeriod;
        }
    };

    template <class T_parameter>
    struct PingPong
    {
        T_parameter operator()(T_parameter aPeriod, T_parameter aAbsoluteValue) const
        {
            T_parameter doubleDuration = 2 * aPeriod;
            T_parameter doublePeriod =
                aAbsoluteValue - std::floor(aAbsoluteValue/doubleDuration) * doubleDuration;

            return doublePeriod - 2 * std::max(T_parameter{0}, doublePeriod - aPeriod);
        }
    };
} // namespace periodicity


enum AnimationResult
{
    FullRange = 0,
    Clamped,
};


template <class T_parameter, AnimationResult N_resultRange>
class ParameterAnimation_ResultHelper
{
public:
    using Result_type = T_parameter;

protected:
    ParameterAnimation_ResultHelper(T_parameter aSpeed) :
        mSpeed{std::move(aSpeed)}
    {}

    T_parameter mSpeed;
};


// Specialization when the result is to be clamped.
template <class T_parameter>
class ParameterAnimation_ResultHelper<T_parameter, Clamped>
{
    // TODO remove math::
    using Result_type = math::Clamped<T_parameter>;

protected:
    static constexpr T_parameter mSpeed{1};
};


/// \brief An empty class template.
template <class>
struct None
{};


// TODO potential optimization (only speed or period member) for no-easing/no-periodicity
template <class T_parameter,
          AnimationResult  N_resultRange = FullRange,
          template <class> class TT_periodicity = None,
          template <class> class TT_easeFunctor = None>
class ParameterAnimation : ParameterAnimation_ResultHelper<T_parameter, N_resultRange>
{
    using Base_t = ParameterAnimation_ResultHelper<T_parameter, N_resultRange>;

public:
    template <bool N_isClamped = IsClamped()>
    explicit ParameterAnimation(T_parameter aPeriod, std::enable_if_t<N_isClamped>* = nullptr) :
        mPeriod{aPeriod}
    {}

    template <bool N_isClamped = IsClamped()>
    explicit ParameterAnimation(T_parameter aPeriod, T_parameter aSpeed = T_parameter{1},
                                std::enable_if_t<!N_isClamped>* = nullptr) :
        Base_t{aSpeed},
        mPeriod{aPeriod}
    {}


    // Note: This is trying its best to implement the overall animation logic below:
    //   amplitude * easeFunctor(normalize(periodicBehaviour(speedFactor * aInput)));
    // while statically disabling what is not relevant in the specific template instantiation.
    Result_type at(T_parameter aInput) const
    {
        aInput *= mSpeed;

        if constexpr(IsPeriodic())
        {
            aInput = mPeriodicBehaviour(mPeriod, aInput);
        }

        if constexpr(IsEasing())
        {
            static_assert(!std::is_integral_v<T_parameter>,
                          "Integral parameters require more work regarding normalization.");

            // Need to normalize the easing input
            aInput = TT_easeFunctor<T_parameter>::ease(aInput / mPeriod);

            if constexpr(N_resultRange == FullRange)
            {
                // Then expand it back to the full amplitude if result is not full range
                aInput *= mPeriod;
            }
        }

        return aInput;
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
    static constexpr bool IsEasing()
    {
        return !std::is_same_v<TT_easeFunctor<void>, None<void>>;
    }

    static constexpr bool IsPeriodic()
    {
        return !std::is_same_v<TT_periodicity<void>, None<void>>;
    }

    static constexpr bool IsClamped()
    {
        return N_resultRange == Clamped;
    }

    T_parameter mPeriod;
    T_parameter mAccumulatedInput{0};
    TT_periodicity<T_parameter> mPeriodicBehaviour; // empty class for basic initial cases (Repeat, PingPong)
                                                    // but leaves room for more potential other scenarios.
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
            arte::AnimationSpriteSheet::LoadAseFile(resource::pathFor("animations/" + gAnimationName + ".json"));

        mAnimator.load(sheet, mSpriting);

        // Animation is given in milliseconds, so multiply speed by 10^3
        mAnimationParameter =
            ParameterAnimation_t{
                mAnimator.get(gAnimationName).totalDuration,
                200
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
        //ParameterAnimation<double, FullRange>;
        ParameterAnimation<double, FullRange, periodicity::Repeat>;
        //ParameterAnimation<double, FullRange, periodicity::PingPong>;
        //ParameterAnimation<double, FullRange, periodicity::Repeat, math::ease::SmoothStep>;
        //ParameterAnimation<double, FullRange, periodicity::PingPong, math::ease::SmoothStep>;
        //ParameterAnimation<double, FullRange, None, math::ease::SmoothStep>;

    Spriting mSpriting;
    Animator mAnimator;
    std::optional<ParameterAnimation_t> mAnimationParameter;
};


} // namespace graphics
} // namespace ad
