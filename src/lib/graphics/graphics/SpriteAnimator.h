#pragma once


#include "Sprite.h"

#include <arte/SpriteSheet.h>

#include <math/Color.h>


namespace ad {
namespace graphics {


// Forward
class Spriting;


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
    /// \brief Load a sprite sheet into a `Spriting` renderer.
    /// This means the renderer can later draw frames for this animation.
    void load(const arte::AnimationSpriteSheet & aSpriteSheet, Spriting & aSpriting);

    /// \brief Overload loading several sprite sheets into the `Spriting` renderer.
    template <class T_iterator>
    void load(T_iterator aSheetBegin, T_iterator aSheetEnd, Spriting & aSpriting);

    /// \brief Retrieves an `Animation` from its identifier. 
    const Animation & get(const std::string & aAnimationId) const
    { return mAnimations.at(aAnimationId); }

    /// \brief Retrieve an `Animation` frame directly.
    LoadedSprite at(const std::string & aAnimationId, Animation::Duration_t aAnimationTime) const
    { return get(aAnimationId).at(aAnimationTime); }

private:
    /// Prepare the frames in `aSpriteSheet` to be renderable from `aSpriting`, but does not
    /// load any texture (this must be done separately).
    void insertAnimationFrames(const arte::AnimationSpriteSheet & aSpriteSheet, 
                               math::Vec<2, int> aTextureOffset,
                               Spriting & aSpriting);

    std::map<std::string, Animation> mAnimations;
};


//
// Implementations
//
template <class T_iterator>
void Animator::load(T_iterator aSheetBegin, T_iterator aSheetEnd, Spriting & aSpriting)
{
    // compute the required atlas size
    math::Size<2, int> atlasResolution = math::Size<2, int>::Zero();
    for (T_iterator sheetIt = aSheetBegin; sheetIt != aSheetEnd; ++sheetIt)
    {
        // TODO that would make a nice factorized function
        atlasResolution.width() = std::max(atlasResolution.width(), sheetIt->image().dimensions().width());
        atlasResolution.height() += sheetIt->image().dimensions().height();
    }

    // assemble the atlas from all sprite sheet
    // TODO might be optimized by writing directly into a texture of the appropriate dimension.
    arte::Image<math::sdr::Rgba> spriteAtlas{atlasResolution, math::sdr::gTransparent};
    math::Vec<2, int> offset{0, 0};
    for (T_iterator sheetIt = aSheetBegin; sheetIt != aSheetEnd; ++sheetIt)
    {
        spriteAtlas.pasteFrom(sheetIt->image(), offset.as<math::Position>());
        // Also prepare the animation frames, as they do not require the texture to be loaded already.
        insertAnimationFrames(*sheetIt, offset, aSpriting);
        offset.y() += sheetIt->image().dimensions().height();
    }

    // load the atlas as a texture into spriting renderer
    aSpriting.prepareTexture(spriteAtlas);
}


} // namespace sprite
} // namespace graphics 
} // namespace ad
