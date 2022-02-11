#include "SpriteLoading.h"

#include <arte/SpriteSheet.h>


namespace ad {
namespace graphics {
namespace sprite {


SheetLoad loadMetaFile(const filesystem::path & aPath)
{
    arte::TileSheet sheet = arte::TileSheet::LoadMetaFile(aPath);
    return load(sheet.cbegin(), sheet.cend(), sheet.image());
}


} // namespace sprite
} // namespace graphics
} // namespace ad
