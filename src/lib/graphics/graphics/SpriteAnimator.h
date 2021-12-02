#pragma once


#include "Sprite.h"

#include <arte/SpriteSheet.h>


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
    std::map<std::string, Animation> mAnimations;
};


//
// Implementations
//
template <class T_iterator>
void load(T_iterator aSheetBegin, T_iterator aSheetEnd, Spriting & aSpriting)
{
    static_assert(false, "Not implemented.");
}


} // namespace sprite
} // namespace graphics
} // namespace ad
