#pragma once

#include <renderer/Rectangle.h>

#include <math/Vector.h>

#include <string>

namespace ad {

struct Sprite
{
    const std::string mName;
    const Rectangle mTextureArea;
};

} // namespace ad
