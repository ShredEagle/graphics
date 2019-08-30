#pragma once

#include <math/Vector.h>

namespace ad {

/// \todo Move to the Math library
struct Rectangle
{
    math::Vec2<int> mPosition;
    math::Dimension2<int> mDimension;

    int x() const
    { return mPosition.x(); };
    int y() const
    { return mPosition.y(); };

    int width() const
    { return mDimension.width(); }
    int height() const
    { return mDimension.height(); }
};

} // namespace ad
