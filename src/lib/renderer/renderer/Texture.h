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

    /// \return First position after last written element
    unsigned char * cropTo(unsigned char * aDestination, const Rectangle aZone) const
    {
        int startOffset = (aZone.y()*mDimension.width() + aZone.x());
        for (int line = 0; line != aZone.height(); ++line)
        {
            aDestination = std::copy(mResource + (startOffset)*gComponents,
                                     mResource + (startOffset + aZone.width())*gComponents,
                                     aDestination);
            startOffset += mDimension.width();
        }
        return aDestination;
    }

    Image crop(const Rectangle aZone) const
    {
        std::unique_ptr<unsigned char[]> target{
            new unsigned char[aZone.mDimension.area() * gComponents]
        };

        cropTo(target.get(), aZone);

        return {target.release(), aZone.mDimension, mSourceComponents};
    }

    Image prepareArray(std::vector<math::Vec2<int>> aPositions,
                       math::Dimension2<int> aDimension) const
    {
        std::unique_ptr<unsigned char[]> target{
            new unsigned char[aDimension.area() * gComponents * aPositions.size()]
        };

        unsigned char * destination = target.get();
        for(const auto position : aPositions)
        {
            destination = cropTo(destination, {position, aDimension});
        }

        return {
            target.release(),
            {static_cast<int>(aDimension.width() * aPositions.size()), aDimension.height()},
            mSourceComponents
        };
    }

    std::vector<Image> cutouts(std::vector<math::Vec2<int>> aPositions,
                               math::Dimension2<int> aDimension) const
    {
        std::vector<Image> cutouts;
        for(const auto position : aPositions)
        {
            cutouts.push_back(crop({position, aDimension}));
        }
        return cutouts;
    }

    math::Dimension2<int> mDimension;
    /// \brief The number of channels in the source image, not in the current data
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
