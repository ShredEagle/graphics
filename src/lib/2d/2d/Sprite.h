#pragma once

#include <renderer/Image.h>
#include <renderer/Rectangle.h>

#include <math/Vector.h>

#include <string>
#include <vector>

namespace ad {

struct Sprite
{
    const std::string mName;
    const Rectangle mTextureArea;
};

struct SpriteSheet
{
    std::vector<Sprite> mSprites;
    Image mRasterData;
};

// Implementer's note: Leave room for potential future optimization, by changing this type
struct LoadedSprite : public Sprite
{};

} // namespace ad
