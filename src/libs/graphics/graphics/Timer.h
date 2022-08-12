#pragma once

namespace ad {
namespace graphics {

struct Timer
{
    void mark(double aMonotonic)
    {
        mDelta = aMonotonic - mTime;
        mTime = aMonotonic;
    }

    double delta() const
    {
        return mDelta;
    }

    double time() const
    {
        return mTime;
    }

    double mTime;
    double mDelta;
};

} // namespace graphics
} // namespace ad
