#pragma once

#include "commons.h"

#include "Sprite.h"

#include <renderer/Drawing.h>
#include <vector>


namespace ad {
namespace graphics {


/// \brief Draws a list of sprites (loaded from a single spritesheet with ::load()) at given positions.
///
/// The instance data is a span of association between a (rendering) position and a sprite (in the spritesheet).
class Spriting
{
public:
    struct Instance
    {
        Instance(Position2<GLint> aRenderingPosition, LoadedSprite aSprite, GLfloat aOpacity = 1.f):
            mPosition{std::move(aRenderingPosition)},
            mLoadedSprite{std::move(aSprite)},
            mOpacity{aOpacity}
        {}

        Position2<GLint> mPosition;
        LoadedSprite mLoadedSprite;
        GLfloat mOpacity;
    };

    Spriting(Size2<int> aRenderResolution);

    /// \brief Takes a pair of iterator to SpriteArea instances, and the corresponding raster data
    template <class T_iterator>
    std::vector<LoadedSprite> load(T_iterator aFirst, T_iterator aLast,
                                   const Image & aRasterData);

    /// \brief Load the entire image as a single sprite.
    LoadedSprite load(const Image & aRasterData);

    void render(gsl::span<const Instance> aInstances) const;

    void setBufferResolution(Size2<int> aNewResolution);

private:
    DrawContext mDrawContext;
};


//
// Implementation
//

template <class T_iterator>
std::vector<LoadedSprite> Spriting::load(T_iterator aFirst, T_iterator aLast,
                                         const Image & aRasterData)
{
    static_assert(std::is_convertible_v<decltype(*std::declval<T_iterator>()), SpriteArea>,
        "Iterators must point to SpriteArea instances.");

    { // scope texture
        Texture texture{GL_TEXTURE_RECTANGLE};
        loadSpriteSheet(texture, GL_TEXTURE0, aRasterData, aRasterData.dimension());
        mDrawContext.mTextures.push_back(std::move(texture));
    }

    std::vector<LoadedSprite> loadedSprites;
    std::copy(aFirst, aLast, std::back_inserter(loadedSprites));
    return loadedSprites;
}


inline LoadedSprite Spriting::load(const Image & aRasterData)
{
    std::initializer_list<SpriteArea> fullSize{ {{0, 0}, aRasterData.dimension()} };
    return load(fullSize.begin(), fullSize.end(), aRasterData).at(0);
}


} // namespace graphics
} // namespace ad
