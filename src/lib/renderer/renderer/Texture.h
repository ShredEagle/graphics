#pragma once

#include "commons.h"
#include "gl_helpers.h"
#include "MappedGL.h"

#include <arte/Image.h>

#include <handy/Guard.h>

#include "GL_Loader.h"

#include <cassert>
#include <string>

namespace ad {
namespace graphics {

namespace detail {

/// \brief Set the alignment of subsequent texture unpack (write) operation to `aAlignment`,
/// then restore the previous alignment on returned guard destruction.
inline Guard scopeUnpackAlignment(GLint aAlignment)
{
    GLint previousAlignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, aAlignment);
    return Guard{ [previousAlignment](){glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);} };
}

} // namespace detail

struct [[nodiscard]] Texture : public ResourceGuard<GLuint>
{
    Texture(GLenum aTarget) :
        ResourceGuard<GLuint>(reserve(glGenTextures),
                              [](GLuint textureId){glDeleteTextures(1, &textureId);}),
        mTarget(aTarget)
    {}

    GLenum mTarget;
};

inline void bind(const Texture & aTexture)
{
    glBindTexture(aTexture.mTarget, aTexture);
}

inline void unbind(const Texture & aTexture)
{
    glBindTexture(aTexture.mTarget, 0);
}

inline void bind(const Texture & aTexture, GLenum aTextureUnit)
{
    glActiveTexture(aTextureUnit);
    bind(aTexture);
}

inline void unbind(const Texture & aTexture, GLenum aTextureUnit)
{
    glActiveTexture(aTextureUnit);
    unbind(aTexture);
}

inline void setFiltering(const Texture & aTexture, GLenum aFiltering)
{
    bind_guard scoped{aTexture};
    glTexParameteri(aTexture.mTarget, GL_TEXTURE_MIN_FILTER, aFiltering);
    glTexParameteri(aTexture.mTarget, GL_TEXTURE_MAG_FILTER, aFiltering);
}

/// \brief Allocate texture storage.
inline void allocateStorage(const Texture & aTexture, const GLenum aInternalFormat,
                            const GLsizei aWidth, const GLsizei aHeight)
{
    bind_guard bound(aTexture);

    if (GL_ARB_texture_storage)
    {
        glTexStorage2D(aTexture.mTarget, 1, aInternalFormat, aWidth, aHeight);
        { // scoping isSuccess
            GLint isSuccess;
            glGetTexParameteriv(aTexture.mTarget, GL_TEXTURE_IMMUTABLE_FORMAT, &isSuccess);
            if ( isSuccess != GL_TRUE)
            {
                const std::string message{"Error calling 'glTexStorage2D'"};
                std::cerr << message << std::endl;
                throw std::runtime_error(message);
            }
        }
    }
    else
    {
        throw std::runtime_error("Not implemented" + std::to_string(__LINE__));
    }
}

inline void allocateStorage(const Texture & aTexture, const GLenum aInternalFormat,
                            math::Size<2, GLsizei> aResolution)
{
    return allocateStorage(aTexture, aInternalFormat,
                           aResolution.width(), aResolution.height());
}

/// \brief Parameter for `writeTo()` below.
struct InputImageParameters
{
    math::Size<2, GLsizei> resolution;
    GLenum format;
    GLenum type;
    GLint alignment; // maps to GL_UNPACK_ALIGNMENT
};

/// \brief OpenGL pixel unpack operation, writing to a texture whose storage is already allocated.
inline void writeTo(const Texture & aTexture,
                    const std::byte * aRawData,
                    const InputImageParameters & aInput,
                    math::Position<2, GLint> aTextureOffset = {0, 0},
                    GLint aMipmapLevel = 0)
{
    bind_guard bound(aTexture);

    // Handle alignment
    Guard scopedAlignemnt = detail::scopeUnpackAlignment(aInput.alignment);

    glTexSubImage2D(aTexture.mTarget, aMipmapLevel,
                    aTextureOffset.x(), aTextureOffset.y(),
                    aInput.resolution.width(), aInput.resolution.height(),
                    aInput.format, aInput.type,
                    aRawData);
}

/// \brief Allocate storage and read `aImage` into `aTexture`.
template <class T_pixel>
inline void loadImage(const Texture & aTexture,
                      const arte::Image<T_pixel> & aImage)
{
    // Probably too restrictive
    assert(aTexture.mTarget == GL_TEXTURE_2D 
        || aTexture.mTarget == GL_TEXTURE_RECTANGLE);

    allocateStorage(aTexture, GL_RGBA8, aImage.dimensions());
    writeTo(aTexture, static_cast<const std::byte *>(aImage), 
            { aImage.dimensions(), MappedPixel_v<T_pixel>, GL_UNSIGNED_BYTE, (GLint)aImage.rowAlignment() });
}

/// \brief Load an animation from an image containing a (column) array of frames.
template <class T_pixel>
inline void loadAnimationAsArray(const Texture & aTexture,
                                 GLenum aTextureUnit,
                                 const arte::Image<T_pixel> & aImage,
                                 const Size2<int> & aFrame,
                                 size_t aSteps)
{
    // Implementers note:
    // This implemention is kept with the "old-approach" to illustrate
    // how it can be done pre GL_ARB_texture_storage

    assert(aTexture.mTarget == GL_TEXTURE_2D_ARRAY);

    bind_guard bound(aTexture);

    Guard scopedAlignemnt = detail::scopeUnpackAlignment(aImage.rowAlignment());

    glTexImage3D(aTexture.mTarget, 0, GL_RGBA,
                 aFrame.width(), aFrame.height(), static_cast<GLsizei>(aSteps),
                 0, MappedPixel_v<T_pixel>, GL_UNSIGNED_BYTE, static_cast<const unsigned char *>(aImage));

    // Texture parameters
    glTexParameteri(aTexture.mTarget, GL_TEXTURE_MAX_LEVEL, 0);
    // Sampler parameters
    glTexParameteri(aTexture.mTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(aTexture.mTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}


} // namespace graphics
} // namespace ad
