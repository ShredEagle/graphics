#pragma once


#include "Image.h"

#include <platform/Filesystem.h>

#include <math/Rectangle.h>

#include <string>
#include <vector>


namespace ad {
namespace arte {


class SpriteSheet
{
public:
    using Duration_t = float;
    using SpriteArea = math::Rectangle<int>;
    
    template <class T_area>
    struct Frame_base
    {
        // Usefull for loading into Spriting renderer, as it expects iterators to SpriteArea.
        /*implicit*/operator const SpriteArea & () const
        { return area; }

        std::string name;
        Duration_t duration;
        T_area area;
    };

    using Frame = Frame_base<SpriteArea>;

    static SpriteSheet LoadAseFile(const filesystem::path & aJsonData);

    const std::string & name() const
    { return mName; }

    // Note: return by const reference to error on assignment
    float scale() const
    { return mScale; }

    Duration_t totalDuration() const
    { return mTotalDuration; }

    std::size_t frameCount() const
    { return mFrames.size(); }

    const Image<> image() const
    { return mSheet; }

    auto beginFrames() const
    { return mFrames.begin(); }

    auto endFrames() const
    { return mFrames.end(); }

private:
    SpriteSheet(std::string aName, const filesystem::path & aSheetImage);

    std::string mName;
    float mScale{1};
    Duration_t mTotalDuration{0};
    Image<> mSheet;
    std::vector<Frame> mFrames;
};



} // namespace arte
} // namespace ad
