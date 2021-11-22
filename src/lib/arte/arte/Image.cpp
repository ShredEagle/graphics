#include "Image.h"

#include "detail/ImageFormats/Netpbm.h"
#include "detail/ImageFormats/StbImageFormats.h"

#include <algorithm>
#include <string>
#include <istream>
#include <sstream>

namespace ad {
namespace arte {


template <class T_pixelFormat>
Image<T_pixelFormat>::Image(math::Size<2, int> aDimensions, std::unique_ptr<unsigned char[]> aRaster) :
    mDimensions{aDimensions},
    mRaster{std::move(aRaster)}
{}


template <class T_pixelFormat>
Image<T_pixelFormat>::Image(math::Size<2, int> aDimensions, T_pixelFormat aBackgroundValue) :
    mDimensions{aDimensions},
    mRaster{new unsigned char[mDimensions.area() * pixel_size_v]}
{
    std::fill(begin(), end(), aBackgroundValue);
}


template <class T_pixelFormat>
Image<T_pixelFormat>::Image(const Image & aRhs) :
    mDimensions(aRhs.mDimensions),
    mRaster{new unsigned char[mDimensions.area() * pixel_size_v]}
{
    std::copy(aRhs.begin(), aRhs.end(), begin());
}


template <class T_pixelFormat>
Image<T_pixelFormat> & Image<T_pixelFormat>::operator=(const Image & aRhs)
{
    *this = Image{aRhs};
    return *this;
}


template <class T_pixelFormat>
Image<T_pixelFormat>::Image(const filesystem::path & aImageFile,
                            ImageOrientation aOrientation) :
    Image{LoadFile(aImageFile, aOrientation)}
{}


template <>
void Image<math::sdr::Rgb>::write(ImageFormat aFormat, std::ostream & aOut,
                                  ImageOrientation aOrientation) const
{
    switch(aFormat)
    {
    case ImageFormat::Ppm:
        detail::Netpbm<detail::NetpbmFormat::Ppm>::Write(aOut, *this, aOrientation);
        break;
    default:
        throw std::runtime_error{"Unsupported write format for RGB image: "
                                 + to_string(aFormat)};
    }
}


template <>
void Image<math::sdr::Rgba>::write(ImageFormat aFormat, std::ostream & aOut,
                                   ImageOrientation aOrientation) const
{
    throw std::runtime_error{"Writing RGBA image is not implemented."};
}


template <>
void Image<math::sdr::Grayscale>::write(ImageFormat aFormat, std::ostream & aOut,
                                        ImageOrientation aOrientation) const
{
    switch(aFormat)
    {
    case ImageFormat::Pgm:
        detail::Netpbm<detail::NetpbmFormat::Pgm>::Write(aOut, *this, aOrientation);
        break;
    default:
        throw std::runtime_error{"Unsupported write format for grayscale image: "
                                 + to_string(aFormat)};
    }
}


template <>
Image<math::sdr::Rgb> Image<math::sdr::Rgb>::Read(ImageFormat aFormat,
                                                  std::istream & aIn,
                                                  ImageOrientation aOrientation)
{
    switch(aFormat)
    {
    case ImageFormat::Ppm:
        return detail::Netpbm<detail::NetpbmFormat::Ppm>::Read(aIn, aOrientation);
    case ImageFormat::Png:
        return detail::StbImageFormats::Read<math::sdr::Rgb>(aIn, aOrientation);
    case ImageFormat::Bmp:
        return detail::StbImageFormats::Read<math::sdr::Rgb>(aIn, aOrientation);
    default:
        throw std::runtime_error{"Unsupported read format to produce and RGB image: "
                                 + to_string(aFormat)};
    }
}


template <>
Image<math::sdr::Rgba> Image<math::sdr::Rgba>::Read(ImageFormat aFormat,
                                                    std::istream & aIn,
                                                    ImageOrientation aOrientation)
{
    // Important: PPM standard does **not** support a transparency channel.
    switch(aFormat)
    {
    case ImageFormat::Png:
        return detail::StbImageFormats::Read<math::sdr::Rgba>(aIn, aOrientation);
    case ImageFormat::Bmp:
        return detail::StbImageFormats::Read<math::sdr::Rgba>(aIn, aOrientation);
    default:
        throw std::runtime_error{"Unsupported read format to produce and RGBA image: "
                                 + to_string(aFormat)};
    }
}


template <>
Image<math::sdr::Grayscale> Image<math::sdr::Grayscale>::Read(ImageFormat aFormat,
                                                              std::istream & aIn,
                                                              ImageOrientation aOrientation)
{
    switch(aFormat)
    {
    case ImageFormat::Pgm:
        return detail::Netpbm<detail::NetpbmFormat::Pgm>::Read(aIn, aOrientation);
    default:
        throw std::runtime_error{"Unsupported read format for grayscale image: "
                                 + to_string(aFormat)};
    }
}


template <class T_pixelFormat>
Image<T_pixelFormat> Image<T_pixelFormat>::LoadFile(const filesystem::path & aImageFile,
                                                    ImageOrientation aOrientation)
{
    return Read(from_extension(aImageFile.extension()),
                std::ifstream{aImageFile.string(), std::ios_base::in | std::ios_base::binary},
                aOrientation);
}


template <class T_pixelFormat>
void Image<T_pixelFormat>::saveFile(const filesystem::path & aDestination, ImageOrientation aOrientation) const
{
    write(from_extension(aDestination.extension()),
          std::ofstream{aDestination.string(), std::ios_base::out | std::ios_base::binary},
          aOrientation);
}


template <class T_pixelFormat>
void Image<T_pixelFormat>::clear(T_pixelFormat aClearColor)
{
    // TODO is there a more efficient approach?
    std::fill(begin(), end(), aClearColor);
}


template <class T_pixelFormat>
Image<T_pixelFormat> Image<T_pixelFormat>::crop(const math::Rectangle<int> & aZone) const
{
    auto destination = std::make_unique<unsigned char []>(aZone.mDimension.area() * pixel_size_v);
    cropTo(reinterpret_cast<T_pixelFormat *>(destination.get()), aZone);
    return {aZone.dimension(), std::move(destination)};
}


template <class T_pixelFormat>
T_pixelFormat * Image<T_pixelFormat>::cropTo(T_pixelFormat * aDestination, const math::Rectangle<int> & aZone) const
{
    std::size_t startOffset = aZone.y() * dimensions().width() + aZone.x();
    for (int line = 0; line != aZone.height(); ++line)
    {
        aDestination = std::copy(data() + startOffset,
                                 data() + (startOffset + aZone.width()),
                                 aDestination);
        startOffset += dimensions().width();
    }
    return aDestination;
}


Image<math::sdr::Grayscale> toGrayscale(const Image<math::sdr::Rgb> & aSource)
{
    auto destination = std::make_unique<unsigned char[]>(aSource.dimensions().area());

    std::transform(aSource.begin(), aSource.end(), destination.get(),
                   [](math::sdr::Rgb aPixel) -> math::sdr::Grayscale
                   {
                       return (aPixel.r() + aPixel.g() + aPixel.b()) / 3;
                   });

    return {aSource.dimensions(), std::move(destination)};
}


//
// Explicit instantiations
//
template class Image<math::sdr::Rgb>;
template class Image<math::sdr::Rgba>;
template class Image<math::sdr::Grayscale>;


} // namespace arte
} // namespace ad
