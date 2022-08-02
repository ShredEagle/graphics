#include "SpriteLoading.h"

#include <arte/SpriteSheet.h>


namespace ad {
namespace graphics {
namespace sprite {


SheetLoad load(const arte::TileSheet & aTileSheet)
{
    return load(aTileSheet.cbegin(), aTileSheet.cend(), aTileSheet.image());
}


SheetLoad loadMetaFile(const filesystem::path & aPath)
{
    return load(arte::TileSheet::LoadMetaFile(aPath));
}


} // namespace sprite
} // namespace graphics
} // namespace ad
