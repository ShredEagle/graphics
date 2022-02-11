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
// (What I had in mind is a single index into a uniform array containing the different sprite areas.
// This uniform array would become part of the LoadedAtlas.)
// TODO Ad 2022/02/10: Should it also keep a reference to the texture / the loaded atlas?
// This way it would be more autonomous, instead of having clients maintain the relationship on their own.
// On the other hand, it could break the API approach currently taken in Tiling.
// (i.e. Tiling::render(const sprite::LoadedAtlas & aAtlas, const TileSet & aTileSet))
// And grow the loadedsprite significantly (with a lot of duplication in container of animation frames...).
using LoadedSprite = SpriteArea;


} // namespace graphics
} // namespace ad
