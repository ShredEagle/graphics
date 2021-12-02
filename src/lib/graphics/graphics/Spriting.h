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

    /// \brief Take a pair of iterator to SpriteArea instances, and the corresponding raster data.
    /// Invoke a callback for each frame.
    template <class T_iterator, class T_pixel>
    void loadCallback(T_iterator aFirst, T_iterator aLast,
                      const arte::Image<T_pixel> & aRasterData,
                      std::function<void(const LoadedSprite &,
                                         const typename std::iterator_traits<T_iterator>::value_type &)>);

    /// \brief Load the entire image as a single sprite.
    /// \return a vector of LoadedSprites.
    template <class T_iterator, class T_pixel>
    std::vector<LoadedSprite> load(T_iterator aFirst, T_iterator aLast,
                                   const arte::Image<T_pixel> & aRasterData);

    /// \brief Load the entire image as a single sprite.
    template <class T_pixel>
    LoadedSprite load(const arte::Image<T_pixel> & aRasterData);

    void updateInstances(gsl::span<const Instance> aInstances);

    void render() const;

    void setBufferResolution(Size2<int> aNewResolution);

    void setCameraTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation);
    void setProjectionTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation);


    static constexpr GLint gTextureUnit{0};

private:
    DrawContext mDrawContext;
    GLsizei mInstanceCount{0};
};


//
// Implementation
//

template <class T_iterator, class T_pixel>
void Spriting::loadCallback(T_iterator aFirst, T_iterator aLast,
                            const arte::Image<T_pixel> & aRasterData,
                            std::function<void(const LoadedSprite &,
                                               const typename std::iterator_traits<T_iterator>::value_type &)> aFrameLoadCallback)
{
    static_assert(std::is_convertible_v<decltype(*std::declval<T_iterator>()), SpriteArea>,
                  "Iterators must point to SpriteArea instances.");

    // Only support a single texture at the moment.
    mDrawContext.mTextures.clear();

    { // scope texture
        Texture texture{GL_TEXTURE_RECTANGLE};
        loadImage(texture, aRasterData);
        mDrawContext.mTextures.push_back(std::move(texture));
        setFiltering(texture, GL_NEAREST);
    }

    for (; aFirst != aLast; ++aFirst)
    {
        // for now, LoadedSprite == SpriteArea
        aFrameLoadCallback(static_cast<LoadedSprite>(*aFirst), *aFirst);
    }
}


template <class T_iterator, class T_pixel>
std::vector<LoadedSprite> Spriting::load(T_iterator aFirst, T_iterator aLast,
                                         const arte::Image<T_pixel> & aRasterData)
{
    std::vector<LoadedSprite> result;
    loadCallback(aFirst, aLast, aRasterData,
        [&](const auto & loadedSprite, auto)
        {
            result.push_back(loadedSprite);
        }
    );
    return result;
}


template <class T_pixel>
LoadedSprite Spriting::load(const arte::Image<T_pixel> & aRasterData)
{
    std::initializer_list<SpriteArea> fullSize{ {{0, 0}, aRasterData.dimensions()} };
    return load(fullSize.begin(), fullSize.end(), aRasterData).at(0);
}


} // namespace graphics
} // namespace ad
