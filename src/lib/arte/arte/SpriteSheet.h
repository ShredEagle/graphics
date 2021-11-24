#pragma once


#include "Image.h"

#include <platform/Filesystem.h>

#include <math/Rectangle.h>

#include <string>
#include <vector>


namespace ad {
namespace arte {


using SpriteArea = math::Rectangle<int>;


template <class T_area>
struct Frame_base
{
    // Usefull for loading into Spriting renderer, as it expects iterators to SpriteArea.
    /*implicit*/operator const T_area & () const
    { return area; }

    std::string name;
    T_area area;
};


template <class T_area, class T_duration>
struct AnimationFrame : public Frame_base<T_area>
{
    T_duration duration;
};


template <class T_frame>
class SpriteSheet_base
{
public:
    using Frame = T_frame;
    using const_iterator = typename std::vector<Frame>::const_iterator;

    const std::string & name() const
    { return mName; }

    // Note: return by const reference to error on assignment
    float scale() const
    { return mScale; }

    std::size_t frameCount() const
    { return mFrames.size(); }

    const ImageRgba image() const
    { return mSheet; }

    const_iterator cbegin() const
    { return mFrames.begin(); }

    const_iterator cend() const
    { return mFrames.end(); }

protected:
    SpriteSheet_base(std::string aName, const filesystem::path & aSheetImage);

    std::string mName;
    float mScale{1};
    ImageRgba mSheet;
    std::vector<Frame> mFrames;
};


#define BASE SpriteSheet_base<Frame_base<SpriteArea>>
/// \brief Sprite sheet for animations, i.e. there is a notion of duration attached to each frame.
class TileSheet : public BASE
{
    using Base_t = BASE;
    using Base_t::Base_t;

public:
    static TileSheet LoadMetaFile(const filesystem::path & aJsonData);
};


#undef BASE
#define BASE SpriteSheet_base<AnimationFrame<SpriteArea, float>>
/// \brief Sprite sheet for animations, i.e. there is a notion of duration attached to each frame.
class AnimationSpriteSheet : public BASE
{
    using Base_t = BASE;
    using Base_t::Base_t;

public:
    using Duration_t = decltype(Frame::duration);

    static AnimationSpriteSheet LoadAseFile(const filesystem::path & aJsonData);

    Duration_t totalDuration() const
    { return mTotalDuration; }

private:
    Duration_t mTotalDuration{0};
};
#undef BASE


//
// Implementations
//
template <class T_frame>
SpriteSheet_base<T_frame>::SpriteSheet_base(std::string aName, const filesystem::path & aSheetImage) :
    mName{std::move(aName)},
    mSheet{ImageRgba::LoadFile(aSheetImage, ImageOrientation::InvertVerticalAxis)}
{}


} // namespace arte
} // namespace ad
