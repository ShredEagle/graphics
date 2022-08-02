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


void Animator::insertAnimationFrames(const arte::AnimationSpriteSheet & aSpriteSheet,
                                     math::Vec<2, int> aTextureOffset)
{
    // Should not load empty sprite sheets.
    assert(aSpriteSheet.cbegin() != aSpriteSheet.cend());

    std::vector<Animation::Frame> animationFrames;
    Animation::Duration_t durationAccumulator = 0;
    std::for_each(aSpriteSheet.cbegin(), aSpriteSheet.cend(),
        [&](const arte::AnimationSpriteSheet::Frame & aSourceFrame)
        {
            LoadedSprite loaded = aSourceFrame;
            loaded.origin() += aTextureOffset;

            durationAccumulator += aSourceFrame.duration;
            animationFrames.push_back(Animation::Frame{
                // Frame name is not saved at the moment
                //aSourceFrame.name,
                durationAccumulator,
                loaded
            });
        }
    ); 
    mAnimations.emplace(aSpriteSheet.name(), 
                        Animation{aSpriteSheet.totalDuration(), std::move(animationFrames)});
}


LoadedAtlas Animator::load(const arte::AnimationSpriteSheet & aSpriteSheet)
{
    insertAnimationFrames(aSpriteSheet, {0, 0});
    return loadAtlas(aSpriteSheet.image());
}


} // namespace sprite
} // namespace graphics
} // namespace ad
