#pragma once

#include <renderer/Image.h>
#include <renderer/commons.h>

#include <string>
#include <vector>

namespace ad {

typedef Rectangle<int> SpriteArea;

/// \todo How to address it cleanly? We don't know how client code wants to represent sprites
struct Sprite
{
    const std::string mName;
    const SpriteArea mTextureArea;
};

struct SpriteSheet
{
    std::vector<Sprite> mSprites;
    Image mRasterData;
};

// Implementer's note: Leave room for potential future optimization, by changing this type
typedef SpriteArea LoadedSprite;

} // namespace ad
