#pragma once

#include "commons.h"

#include "Sprite.h"

#include <math/Homogeneous.h>

#include <renderer/Drawing.h>

#include <glad/glad.h>

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
        Instance(Position2<GLfloat> aRenderingPosition, LoadedSprite aSprite, GLfloat aOpacity = 1.f, Vec2<int> aAxisMirroring = {1, 1}):
            mPosition{std::move(aRenderingPosition)},
            mLoadedSprite{std::move(aSprite)},
            mOpacity{aOpacity},
            mAxisMirroring{std::move(aAxisMirroring)}
        {}

        Instance & mirrorHorizontal(bool aMirror = true)
        {
            mAxisMirroring.x() = aMirror ? -1 : 1;
            return *this; 
        }

        Instance & mirrorVertical(bool aMirror = true)
        {
            mAxisMirroring.y() = aMirror ? -1 : 1;
            return *this; 
        }

        Position2<GLfloat> mPosition;
        LoadedSprite mLoadedSprite;
        GLfloat mOpacity;
        Vec2<int> mAxisMirroring;
    };

    Spriting(GLfloat aPixelSize = 1.f);

    //
    // Lower level
    //
    template <class T_pixel>
    void prepareTexture(const arte::Image<T_pixel> & aRasterData);

    template <class T_iterator>
    void prepareSprites(T_iterator aFirst, T_iterator aLast,
                        math::Vec<2, int> aTextureOffset,
                        std::function<void(const LoadedSprite &,
                                           const typename std::iterator_traits<T_iterator>::value_type &)> aFrameLoadCallback);


    //
    // Higher level
    //
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

    /// \brief Define the size of a pixel in world units.
    /// 
    /// When rendering pixel art, it is likely that one sprite pixel should always be the same world size,
    /// independently from the render buffer resolution.
    void setPixelWorldSize(GLfloat aPixelSize);

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

template <class T_pixel>
void Spriting::prepareTexture(const arte::Image<T_pixel> & aRasterData)
{
    // Only support a single texture at the moment.
    mDrawContext.mTextures.clear();

    { // scope texture
        Texture texture{GL_TEXTURE_RECTANGLE};
        loadImage(texture, aRasterData);
        mDrawContext.mTextures.push_back(std::move(texture));
        setFiltering(texture, GL_NEAREST);
    }
}


template <class T_iterator>
void Spriting::prepareSprites(T_iterator aFirst, T_iterator aLast,
                              math::Vec<2, int> aTextureOffset,
                              std::function<void(const LoadedSprite &,
                                                 const typename std::iterator_traits<T_iterator>::value_type &)> aFrameLoadCallback)
{
    static_assert(std::is_convertible_v<decltype(*std::declval<T_iterator>()), SpriteArea>,
                  "Iterators must point to SpriteArea instances.");

    for (; aFirst != aLast; ++aFirst)
    {
        // for now, LoadedSprite == SpriteArea
        LoadedSprite loaded = *aFirst;
        loaded.origin() += aTextureOffset;
        aFrameLoadCallback(loaded, *aFirst);
    }

}


template <class T_iterator, class T_pixel>
void Spriting::loadCallback(T_iterator aFirst, T_iterator aLast,
                            const arte::Image<T_pixel> & aRasterData,
                            std::function<void(const LoadedSprite &,
                                               const typename std::iterator_traits<T_iterator>::value_type &)> aFrameLoadCallback)
{
    prepareTexture(aRasterData);
    prepareSprites(aFirst, aLast, {0, 0}, aFrameLoadCallback);
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
