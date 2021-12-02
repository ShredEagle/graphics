#include "SpriteAnimator.h"

#include "Spriting.h"


namespace ad {
namespace graphics {
namespace sprite {


LoadedSprite Animation::at(Duration_t aLocalTime) const
{
    auto frameIt = frames.begin();
    for (; frameIt != (frames.end() - 1); ++frameIt)
    {
        if (frameIt->endTime >= aLocalTime) break;
    }
    return frameIt->loadedSprite;
}


void Animator::load(const arte::AnimationSpriteSheet & aSpriteSheet, Spriting & aSpriting)
{
    // Should not load empty sprite sheets.
    assert(aSpriteSheet.cbegin() != aSpriteSheet.cend());

    std::vector<Animation::Frame> animationFrames;
    Animation::Duration_t durationAccumulator = 0;
    aSpriting.loadCallback(aSpriteSheet.cbegin(), aSpriteSheet.cend(), aSpriteSheet.image(),
        [&](const LoadedSprite & aLoaded, const arte::AnimationSpriteSheet::Frame & aSourceFrame)
        {
            durationAccumulator += aSourceFrame.duration;
            animationFrames.push_back(Animation::Frame{
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


} // namespace sprite
} // namespace graphics
} // namespace ad
