#pragma once

#include "stb_image.h"

#include <handy/Guard.h>

#include <glad/glad.h>

#include <filesystem>

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

struct Image : public ResourceGuard<unsigned char *>
{
    Image(std::filesystem::path aFilePath) :
        ResourceGuard<unsigned char*>(nullptr, stbi_image_free)
    {
        stbi_set_flip_vertically_on_load(true); // adapt to OpenGL
        mResource = stbi_load(aFilePath.string().c_str(),
                              &mWidth,
                              &mHeight,
                              &mSourceComponents,
                              STBI_rgb_alpha);
    }

    int mWidth;
    int mHeight;
    int mSourceComponents;
};

void loadImageToTexture()
{
}

} // namespace ad
