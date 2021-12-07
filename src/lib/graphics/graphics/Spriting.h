#pragma once

#include "commons.h"

#include "Sprite.h"

#include <math/Homogeneous.h>

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
        Instance(Position2<GLfloat> aRenderingPosition, LoadedSprite aSprite, GLfloat aOpacity = 1.f):
            mPosition{std::move(aRenderingPosition)},
            mLoadedSprite{std::move(aSprite)},
            mOpacity{aOpacity}
        {}

        Position2<GLfloat> mPosition;
        LoadedSprite mLoadedSprite;
        GLfloat mOpacity;
    };

    Spriting(GLfloat aPixelSize = 1.f);

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

    /// \brief Set the size of the viewport in sprite pixels (assuming the default pixel world size of 1).
    ///
    /// This is a helper around `setProjectionTransformation()`: it is setting the world size of the viewport. 
    /// It makes it convenient to work with the virtual pixels as world unit.
    ///
    /// \important the viewport will be [-width/2, -height/2] x [width/2, height/2].
    /// Additionally set a camera transformation if lower-left should be at [0, 0] instead.
    void setViewportVirtualResolution(math::Size<2, int> aViewportPixelSize);

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
