#pragma once

namespace ad {

struct Timer
{
    void mark(double aMonotonic)
    {
        mDelta = aMonotonic - mTime;
        mTime = aMonotonic;
    }

    double mTime;
    double mDelta;
};

} // namespace ad
