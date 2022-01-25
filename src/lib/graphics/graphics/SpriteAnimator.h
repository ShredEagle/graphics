#pragma once


#include "Sprite.h"
#include "SpriteLoading.h"
#include "Spriting.h"

#include <arte/SpriteSheet.h>

#include <handy/StringId.h>

#include <math/Color.h>

#include <unordered_map>


namespace ad {
namespace graphics {


namespace sprite{


/// \brief Model a sprite animation, which is composed of a sequence of frames with their duration.
struct Animation
{
    using Duration_t = arte::AnimationSpriteSheet::Duration_t;

    struct Frame
    {
        Duration_t endTime; // the local end time for this frame
        LoadedSprite loadedSprite;
    };

    /// \brief Obtain the `LoadedSprite` corresponding to local animation time `aLocalTime`.
    ///  
    /// The `LoadedSprite` can then be renderer using `Spriting` renderer.
    ///
    /// \note Please see `math::ParameterAnimation` template to implement
    /// easing and periodicity behaviours.
    LoadedSprite at(Duration_t aLocalTime) const;

    Duration_t totalDuration;
    std::vector<Frame> frames;
};


/// \brief Used to load a collection of sprite sheets into a `Spriting` renderer,
/// then give access to individual `Animations`.
class Animator
{
public:
    /// \brief Load a sprite sheet into the animator.
    /// This means a Spriting renderer can later draw frames for this animation.
    LoadedAtlas load(const arte::AnimationSpriteSheet & aSpriteSheet);

    /// \brief Load several sprite sheets, assembling them into a consolidated atlas.
    template <class T_iterator>
    LoadedAtlas load(T_iterator aSheetBegin, T_iterator aSheetEnd);

    /// \brief Retrieves an `Animation` from its identifier. 
    const Animation & get(const handy::StringId & aAnimationId) const
    { return mAnimations.at(aAnimationId); }

    /// \brief Retrieve an `Animation` frame directly.
    LoadedSprite at(const handy::StringId & aAnimationId, Animation::Duration_t aAnimationTime) const
    { return get(aAnimationId).at(aAnimationTime); }

private:
    /// Prepare the frames in `aSpriteSheet` to be renderable from `aSpriting`, but does not
    /// load any texture (this must be done separately).
    void insertAnimationFrames(const arte::AnimationSpriteSheet & aSpriteSheet, 
                               math::Vec<2, int> aTextureOffset);

    // Note Ad 2021/12:14: It not obvious wether it would be best to use a unordered_map or plain map here.
    std::unordered_map<handy::StringId, Animation> mAnimations;
};


//
// Implementations
//
template <class T_iterator>
LoadedAtlas Animator::load(T_iterator aSheetBegin, T_iterator aSheetEnd)
{
    // compute the required atlas size
    math::Size<2, int> atlasResolution = math::Size<2, int>::Zero();
    for (T_iterator sheetIt = aSheetBegin; sheetIt != aSheetEnd; ++sheetIt)
    {
        // The iterator might not point directly to an AnimationSpriteSheet (e.g. to a reference_wrapper).
        const arte::AnimationSpriteSheet & spriteSheet = *sheetIt;
        // TODO that would make a nice factorized function
        atlasResolution.width() = std::max(atlasResolution.width(), spriteSheet.image().dimensions().width());
        atlasResolution.height() += spriteSheet.image().dimensions().height();
    }

    // assemble the atlas from all sprite sheet
    // TODO might be optimized by writing directly into a texture of the appropriate dimension.
    arte::Image<math::sdr::Rgba> spriteAtlas{atlasResolution, math::sdr::gTransparent};
    math::Vec<2, int> offset{0, 0};
    for (T_iterator sheetIt = aSheetBegin; sheetIt != aSheetEnd; ++sheetIt)
    {
        const arte::AnimationSpriteSheet & spriteSheet = *sheetIt;
        spriteAtlas.pasteFrom(spriteSheet.image(), offset.as<math::Position>());
        // Also prepare the animation frames, as they do not require the texture to be loaded already.
        insertAnimationFrames(spriteSheet, offset);

        offset.y() += spriteSheet.image().dimensions().height();
    }

    // return the loaded atlas
    return loadAtlas(spriteAtlas);
}


} // namespace sprite
} // namespace graphics 
} // namespace ad
