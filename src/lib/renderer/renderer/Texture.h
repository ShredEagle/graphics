#pragma once

#include "commons.h"
#include "gl_helpers.h"

#include <arte/Image.h>

#include <handy/Guard.h>

#include "GL_Loader.h"

#include <cassert>
#include <string>

namespace ad {
namespace graphics {

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


struct InputImageParameters
{
    math::Size<2, GLsizei> resolution;
    GLenum format;
    GLenum type;
    GLint alignment; // maps to GL_UNPACK_ALIGNMENT
};

/// \brief OpenGL pixel unpack operation.
inline void writeTo(const Texture & aTexture,
                    const std::byte * aRawData,
                    const InputImageParameters & aInput,
                    math::Position<2, GLint> aTextureOffset = {0, 0},
                    GLint aMipmapLevel = 0)
{
    bind(aTexture);

    // Handle alignment
    GLint previousAlignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);
    Guard alignment{ [previousAlignment](){glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);} };
    glPixelStorei(GL_UNPACK_ALIGNMENT, aInput.alignment);

    glTexSubImage2D(aTexture.mTarget, aMipmapLevel,
                    aTextureOffset.x(), aTextureOffset.y(),
                    aInput.resolution.width(), aInput.resolution.height(),
                    aInput.format, aInput.type,
                    aRawData);
}

/// \TODO probably useless to activate a texture unit here...
template <class T_pixel>
inline void loadSprite(const Texture & aTexture,
                       GLenum aTextureUnit,
                       const arte::Image<T_pixel> & aImage)
{
    assert(aTexture.mTarget == GL_TEXTURE_2D);

    // TODO remove aTextureUnit from all those calls, it was tested to be not related.
    glActiveTexture(aTextureUnit);
    glBindTexture(aTexture.mTarget, aTexture);

    // TODO factorize
    GLint previousAlignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);
    Guard alignment{ [previousAlignment](){glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);} };
    glPixelStorei(GL_UNPACK_ALIGNMENT, aImage.rowAlignment());

    if (GL_ARB_texture_storage)
    {
        glTexStorage2D(aTexture.mTarget, 1, GL_RGBA8,
                       aImage.dimensions().width(), aImage.dimensions().height());
        {
            GLint isSuccess;
            glGetTexParameteriv(aTexture.mTarget, GL_TEXTURE_IMMUTABLE_FORMAT, &isSuccess);
            if ( isSuccess != GL_TRUE)
            {
                const std::string message{"Error calling 'glTexStorage2D'"};
                std::cerr << message << std::endl;
                throw std::runtime_error(message);
            }
        }
        glTexSubImage2D(aTexture.mTarget, 0,
                        0, 0, aImage.dimensions().width(), aImage.dimensions().height(),
                        MappedPixel_v<T_pixel>, GL_UNSIGNED_BYTE, static_cast<const unsigned char *>(aImage));
    }
    else
    {
        glTexImage2D(aTexture.mTarget, 0, GL_RGBA,
                     aImage.dimensions().width(), aImage.dimensions().height(),
                     0, MappedPixel_v<T_pixel>, GL_UNSIGNED_BYTE, static_cast<const unsigned char *>(aImage));
        // We don't generate mipmaps level,
        // so disable mipmap based filtering for the texture to be complete
        glTexParameteri(aTexture.mTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(aTexture.mTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // Otherwise, we'd generate mipmap levels
        //glGenerateMipmap(GL_TEXTURE_2D);
    }
}

template <class T_pixel>
inline void loadAnimationAsArray(const Texture & aTexture,
                                 GLenum aTextureUnit,
                                 const arte::Image<T_pixel> & aImage,
                                 const Size2<int> & aFrame,
                                 size_t aSteps)
{
    assert(aTexture.mTarget == GL_TEXTURE_2D_ARRAY);

    glActiveTexture(aTextureUnit);
    glBindTexture(aTexture.mTarget, aTexture);

    // TODO factorize
    GLint previousAlignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);
    Guard alignment{ [previousAlignment](){glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);} };
    glPixelStorei(GL_UNPACK_ALIGNMENT, aImage.rowAlignment());

    glTexImage3D(aTexture.mTarget, 0, GL_RGBA,
                 aFrame.width(), aFrame.height(), static_cast<GLsizei>(aSteps),
                 0, MappedPixel_v<T_pixel>, GL_UNSIGNED_BYTE, static_cast<const unsigned char *>(aImage));

    // Texture parameters
    glTexParameteri(aTexture.mTarget, GL_TEXTURE_MAX_LEVEL, 0);
    // Sampler parameters
    glTexParameteri(aTexture.mTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(aTexture.mTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

// Why aFrame when we have dimension in the image?
template <class T_pixel>
inline void loadSpriteSheet(const Texture & aTexture,
                            GLenum aTextureUnit,
                            const arte::Image<T_pixel> & aImage,
                            const Size2<int> & aFrame)
{
    /// \todo can be extended, many other target types are valid here
    assert(aTexture.mTarget == GL_TEXTURE_RECTANGLE);

    glActiveTexture(aTextureUnit);
    glBindTexture(aTexture.mTarget, aTexture);

    GLint previousAlignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);
    Guard alignment{ [previousAlignment](){glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);} };
    glPixelStorei(GL_UNPACK_ALIGNMENT, aImage.rowAlignment());

    glTexImage2D(aTexture.mTarget, 0, GL_RGBA,
                 aFrame.width(), aFrame.height(),
                 0, MappedPixel_v<T_pixel>, GL_UNSIGNED_BYTE, static_cast<const unsigned char *>(aImage));

    // Sampler parameters
    glTexParameteri(aTexture.mTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(aTexture.mTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

} // namespace graphics
} // namespace ad
