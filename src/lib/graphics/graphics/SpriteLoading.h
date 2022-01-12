#pragma once


#include "Sprite.h"

#include <renderer/Texture.h>


namespace ad {
namespace graphics {
namespace sprites {


/// Currently only the texture, but might be extended to contain the buffer of sprite rectangles
/// (so renderer could exped an index into the buffer, instead of complete sprite areas)
struct LoadedAtlas
{
    std::shared_ptr<Texture> texture;
};


/// \brief Takes a pair of iterator to SpriteArea instances, and the corresponding raster data
template <class T_iterator, class T_pixel>
std::pair<LoadedAtlas, std::vector<LoadedSprite>>
load(T_iterator aFirst, T_iterator aLast, const arte::Image<T_pixel> & aRasterData);


std::pair<LoadedAtlas, std::vector<LoadedSprite>> loadMetaFile(const filesystem::path & aPath);

//
// Implementations
//
template <class T_iterator, class T_pixel>
std::pair<LoadedAtlas, std::vector<LoadedSprite>>
load(T_iterator aFirst, T_iterator aLast, const arte::Image<T_pixel> & aRasterData)
{
    static_assert(std::is_convertible_v<decltype(*std::declval<T_iterator>()), SpriteArea>,
                  "Iterators must point to SpriteArea instances.");

    LoadedAtlas atlas{.texture{std::make_shared<Texture>(GL_TEXTURE_RECTANGLE)}};
    loadImage(*atlas.texture, aRasterData);
    setFiltering(*atlas.texture, GL_NEAREST);

    std::vector<LoadedSprite> loadedSprites;
    std::copy(aFirst, aLast, std::back_inserter(loadedSprites));
    return {atlas, loadedSprites};
}


} // namespace sprites
} // namespace graphics
} // namespace ad
