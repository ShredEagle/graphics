#pragma once

#include "stb_image.h"

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

struct Image : public ResourceGuard<unsigned char *>
{
    Image(const std::string & aFilePath) :
        ResourceGuard<unsigned char*>(nullptr, stbi_image_free)
    {
        stbi_set_flip_vertically_on_load(true); // adapt to OpenGL
        mResource = stbi_load(aFilePath.c_str(),
                              &mWidth,
                              &mHeight,
                              &mSourceComponents,
                              STBI_rgb_alpha);

        if (mResource == nullptr)
        {
            const std::string message = "Unable to load image from file '" + aFilePath + "'";
            std::cerr << message;
            throw std::runtime_error(message);
        }
    }

    std::vector<unsigned char> crop(int x, int y, int width, int height) const
    {
        std::vector<unsigned char> target;
        target.reserve(width*height);

        const int startOffset = (y*mWidth + x) * 4;
        for (int line = 0; line != height; ++line)
        {
            target.insert(target.end(),
                          mResource + (startOffset + (line*mWidth)*4),
                          mResource + (startOffset + (line*mWidth + width)*4));
        }

        return target;
    }

    int mWidth;
    int mHeight;
    int mSourceComponents;
};

void loadImageToTexture()
{
}

} // namespace ad
