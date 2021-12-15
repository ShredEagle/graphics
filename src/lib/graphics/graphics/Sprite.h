#pragma once

#include <arte/Image.h>
#include <arte/SpriteSheet.h>

#include <renderer/commons.h>

#include <string>
#include <vector>

namespace ad {
namespace graphics {


// Essentially a cutout in a spritesheet image
using SpriteArea = arte::SpriteArea;

// Implementer's note: Leave room for potential future optimization, by changing this type.
using LoadedSprite = SpriteArea;


} // namespace graphics
} // namespace ad
