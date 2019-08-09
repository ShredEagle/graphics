#pragma once

#include "stb_image.h"

#include <handy/Guard.h>

#include <math/Vector.h>

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


struct Rectangle
{
    math::Vec2<int> mPosition;
    math::Dimension2<int> mDimension;

    int x() const
    { return mPosition.x(); };
    int y() const
    { return mPosition.y(); };

    int width() const
    { return mDimension.width(); }
    int height() const
    { return mDimension.height(); }
};

struct Image : public ResourceGuard<unsigned char *>
{
    Image(const std::string & aFilePath) :
        ResourceGuard<unsigned char*>(nullptr, stbi_image_free),
        mDimension(mDimension.Zero())
    {
        stbi_set_flip_vertically_on_load(true); // adapt to OpenGL
        mResource = stbi_load(aFilePath.c_str(),
                              &mDimension.width(),
                              &mDimension.height(),
                              &mSourceComponents,
                              STBI_rgb_alpha /*equivalent to gComponents*/);

        if (mResource == nullptr)
        {
            const std::string message = "Unable to load image from file '" + aFilePath + "'";
            std::cerr << message;
            throw std::runtime_error(message);
        }
    }

    Image crop(const Rectangle aZone) const
    {
        std::unique_ptr<unsigned char[]> target{
            new unsigned char[aZone.mDimension.area() * gComponents]
        };

        unsigned char * destination = target.get();
        int startOffset = (aZone.y()*mDimension.width() + aZone.x());
        for (int line = 0; line != aZone.height(); ++line)
        {
            destination = std::copy(mResource + (startOffset)*gComponents,
                                    mResource + (startOffset + aZone.width())*gComponents,
                                    destination);
            startOffset += mDimension.width();
        }

        return {target.release(), aZone.mDimension, mSourceComponents};
    }

    //std::vector<std::vector<unsigned char>> cutouts(std::vector<std::pair<int, int>> aCoords, int width, int height) const
    //{
    //    std::vector<std::vector<unsigned char>> cutouts;
    //    for(auto coords : aCoords)
    //    {
    //        cutouts.push_back(crop(coords.first, coords.second, width, height));
    //    }
    //    return cutouts;
    //}

    math::Dimension2<int> mDimension;
    int mSourceComponents;

    static constexpr int gComponents{4};

protected:
    Image(unsigned char * aData,
          math::Dimension2<int> aDimension,
          int aSourceComponents) :
        ResourceGuard<unsigned char*>(aData, [](unsigned char* data){delete [] data;}),
        mDimension(aDimension),
        mSourceComponents(aSourceComponents)
    {}
};

void loadImageToTexture()
{
}

} // namespace ad
