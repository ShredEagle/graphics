#pragma once

#include <handy/Guard.h>

#include <glad/glad.h>

namespace ad
{

struct [[nodiscard]] Texture : public ResourceGuard<GLuint>
{
    GLuint reserveTexture()
    {
        GLuint textureId;
        glGenTextures(1, &textureId);
        return textureId;
    }

    Texture() :
        ResourceGuard<GLuint>(reserveTexture(),
                              [](GLuint textureId){glDeleteTextures(1, &textureId);})
    {}
};

} // namespace ad
