#include "SpriteLoading.h"

#include <arte/SpriteSheet.h>


namespace ad {
namespace graphics {
namespace sprites {


std::pair<LoadedAtlas, std::vector<LoadedSprite>> loadMetaFile(const filesystem::path & aPath)
{
    arte::TileSheet sheet = arte::TileSheet::LoadMetaFile(aPath);
    return load(sheet.cbegin(), sheet.cend(), sheet.image());
}


} // namespace sprites
} // namespace graphics
} // namespace ad
