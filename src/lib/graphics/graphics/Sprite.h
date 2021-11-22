#pragma once

#include <arte/Image.h>
#include <arte/SpriteSheet.h>

#include <renderer/commons.h>

#include <string>
#include <vector>

namespace ad {
namespace graphics {

// Essentially a cutout in a spritesheet image
using SpriteArea = arte::SpriteSheet::SpriteArea;

/// \todo How to address it cleanly? We don't know how client code wants to represent sprites
struct Sprite
{
    const std::string mName;
    const SpriteArea mTextureArea;
};

struct SpriteSheet
{
    std::vector<Sprite> mSprites;
    arte::Image<> mRasterData;
};

// Implementer's note: Leave room for potential future optimization, by changing this type
typedef SpriteArea LoadedSprite;

} // namespace graphics
} // namespace ad
