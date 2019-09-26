#pragma once

#include "gl_helpers.h"
#include "Image.h"

#include <handy/Guard.h>

#include <glad/glad.h>

#include <cassert>

namespace ad
{

struct [[nodiscard]] Texture : public ResourceGuard<GLuint>
{
    Texture(GLenum aTarget) :
        ResourceGuard<GLuint>(reserve(glGenTextures),
                              [](GLuint textureId){glDeleteTextures(1, &textureId);}),
        mTarget(aTarget)
    {}

    const GLenum mTarget;
};


inline void loadSprite(const Texture & aTexture,
                GLenum aTextureUnit,
                const Image & aImage)
{
    assert(aTexture.mTarget == GL_TEXTURE_2D);

    glActiveTexture(aTextureUnit);
    glBindTexture(aTexture.mTarget, aTexture);

    if (GL_ARB_texture_storage)
    {
        glTexStorage2D(aTexture.mTarget, 1, GL_RGBA8,
                       aImage.mDimension.width(), aImage.mDimension.height());
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
                        0, 0, aImage.mDimension.width(), aImage.mDimension.height(),
                        GL_RGBA, GL_UNSIGNED_BYTE, aImage);
    }
    else
    {
        glTexImage2D(aTexture.mTarget, 0, GL_RGBA,
                     aImage.mDimension.width(), aImage.mDimension.height(),
                     0, GL_RGBA, GL_UNSIGNED_BYTE, aImage);
        // We don't generate mipmaps level,
        // so disable mipmap based filtering for the texture to be complete
        glTexParameteri(aTexture.mTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(aTexture.mTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // Otherwise, we'd generate mipmap levels
        //glGenerateMipmap(GL_TEXTURE_2D);
    }
}

inline void loadAnimationAsArray(const Texture & aTexture,
                          GLenum aTextureUnit,
                          const Image & aImage,
                          const Size2<int> & aFrame,
                          size_t aSteps)
{
    assert(aTexture.mTarget == GL_TEXTURE_2D_ARRAY);

    glActiveTexture(aTextureUnit);
    glBindTexture(aTexture.mTarget, aTexture);

    glTexImage3D(aTexture.mTarget, 0, GL_RGBA,
                 aFrame.width(), aFrame.height(), static_cast<GLsizei>(aSteps),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, aImage);

    // Texture parameters
    glTexParameteri(aTexture.mTarget, GL_TEXTURE_MAX_LEVEL, 0);
    // Sampler parameters
    glTexParameteri(aTexture.mTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(aTexture.mTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

inline void loadSpriteSheet(const Texture & aTexture,
                     GLenum aTextureUnit,
                     const Image & aImage,
                     const Size2<int> & aFrame)
{
    /// \todo can be extended, many other target types are valid here
    assert(aTexture.mTarget == GL_TEXTURE_RECTANGLE);

    glActiveTexture(aTextureUnit);
    glBindTexture(aTexture.mTarget, aTexture);

    glTexImage2D(aTexture.mTarget, 0, GL_RGBA,
                 aFrame.width(), aFrame.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, aImage);

    // Sampler parameters
    glTexParameteri(aTexture.mTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(aTexture.mTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

} // namespace ad
