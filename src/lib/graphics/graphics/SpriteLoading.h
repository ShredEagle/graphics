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


/// \brief Overload accepting a range.
template <std::ranges::range T_range, class T_pixel>
std::pair<LoadedAtlas, std::vector<LoadedSprite>>
load(T_range aRange, const arte::Image<T_pixel> & aRasterData);


/// \brief Load all sprites defined in the meta (tilesheet) file.
std::pair<LoadedAtlas, std::vector<LoadedSprite>>
loadMetaFile(const filesystem::path & aPath);


/// \brief Load the entire image as a single sprite.
template <class T_pixel>
std::pair<LoadedAtlas, std::vector<LoadedSprite>>
load(const arte::Image<T_pixel> & aRasterData);


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


template <std::ranges::range T_range, class T_pixel>
std::pair<LoadedAtlas, std::vector<LoadedSprite>>
load(T_range aRange, const arte::Image<T_pixel> & aRasterData)
{
    return load(std::begin(aRange), std::end(aRange), aRasterData);
}


template <class T_pixel>
std::pair<LoadedAtlas, std::vector<LoadedSprite>>
load(const arte::Image<T_pixel> & aRasterData)
{
    std::array<SpriteArea, 1> fullSize{ SpriteArea{{0, 0}, aRasterData.dimensions()} };
    return load(fullSize, aRasterData);
}


} // namespace sprites
} // namespace graphics
} // namespace ad
