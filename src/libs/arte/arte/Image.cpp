#include "Image.h"

#include "detail/ImageFormats/Netpbm.h"
#include "detail/ImageFormats/StbImageFormats.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <istream>
#include <sstream>

#include <cassert>

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
Image<T_pixelFormat>::Image(const filesystem::path & aImageFile,
                            ImageOrientation aOrientation) :
    Image{LoadFile(aImageFile, aOrientation)}
{}


template <class T_pixelFormat>
Image<T_pixelFormat> Image<T_pixelFormat>::makeUninitialized(math::Size<2, int> aDimensions)
{
    return Image{
        aDimensions,
        std::make_unique<unsigned char[]>(sizeof(T_pixelFormat) * aDimensions.area())
    };
}

template <class T_pixelFormat>
Image<T_pixelFormat> & Image<T_pixelFormat>::operator=(const Image & aRhs)
{
    *this = Image{aRhs};
    return *this;
}


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
void Image<math::hdr::Rgb_f>::write(ImageFormat aFormat, std::ostream & aOut,
                                    ImageOrientation aOrientation) const
{
    throw std::runtime_error{"Writing HDR image is not implemented."};
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
    case ImageFormat::Bmp:
        return detail::StbImageFormats::Read<math::sdr::Rgb>(aIn, aOrientation);
    case ImageFormat::Jpg:
        return detail::StbImageFormats::Read<math::sdr::Rgb>(aIn, aOrientation);
    case ImageFormat::Png:
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
    case ImageFormat::Bmp:
        return detail::StbImageFormats::Read<math::sdr::Rgba>(aIn, aOrientation);
    case ImageFormat::Jpg:
        return detail::StbImageFormats::Read<math::sdr::Rgba>(aIn, aOrientation);
    case ImageFormat::Png:
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


template <>
Image<math::hdr::Rgb_f> Image<math::hdr::Rgb_f>::Read(ImageFormat aFormat,
                                                      std::istream & aIn,
                                                      ImageOrientation aOrientation)
{
    switch(aFormat)
    {
    case ImageFormat::Hdr:
        return detail::StbImageFormats::Read<math::hdr::Rgb_f>(aIn, aOrientation);
    default:
        throw std::runtime_error{"Unsupported read format to produce an HDR image: "
                                 + to_string(aFormat)};
    }
}


template <class T_pixelFormat>
Image<T_pixelFormat> Image<T_pixelFormat>::LoadFile(const filesystem::path & aImageFile,
                                                    ImageOrientation aOrientation)
{
    std::ifstream input{aImageFile.string(), std::ios_base::in | std::ios_base::binary};
    assert(input);
    return Read(from_extension(aImageFile.extension()), input, aOrientation);
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


template <class T_pixelFormat>
Image<T_pixelFormat> & Image<T_pixelFormat>::pasteFrom(const Image & aSource, math::Position<2, int> aPastePosition)
{
    int sourceWidth = aSource.dimensions().width();
    if (sourceWidth == dimensions().width())
    {
        // Optimal case: no need to copy row by row
        std::copy(aSource.begin(), aSource.end(), &at(aPastePosition));
    }
    else
    {
        for (int sourceRow = 0; sourceRow != aSource.dimensions().height(); ++sourceRow)
        {
            auto begin = aSource.begin() + sourceRow * sourceWidth;
            std::copy(begin, begin + sourceWidth, &at(aPastePosition.x(), aPastePosition.y() + sourceRow));
        }
    }
    return *this;
}


Image<math::sdr::Grayscale> toGrayscale(const Image<math::sdr::Rgb> & aSource)
{
    auto destination = std::make_unique<unsigned char[]>(aSource.dimensions().area());

    std::transform(aSource.begin(), aSource.end(), destination.get(),
                   [](math::sdr::Rgb aPixel) -> unsigned char
                   {
                        return math::sdr::Grayscale{
                            static_cast<std::uint8_t>((aPixel.r() + aPixel.g() + aPixel.b()) / 3)
                        }.v();
                   });

    return {aSource.dimensions(), std::move(destination)};
}


template <class T_hdrChannel>
Image<math::hdr::Rgb<T_hdrChannel>> to_hdr(const Image<math::sdr::Rgb> & aSource)
{
    auto result = Image<math::hdr::Rgb<T_hdrChannel>>::makeUninitialized(aSource.dimensions());
    std::transform(aSource.begin(), aSource.end(), result.begin(),
                   [](math::sdr::Rgb aSdrPixel) -> math::hdr::Rgb<T_hdrChannel>
                   {
                        return to_hdr<float>(aSdrPixel);
                   });
    return result;
}


template <class T_hdrChannel>
Image<math::sdr::Rgb> tonemap(const Image<math::hdr::Rgb<T_hdrChannel>> & aSource)
{
    auto result = Image<math::sdr::Rgb>::makeUninitialized(aSource.dimensions());
    std::transform(aSource.begin(), aSource.end(), result.begin(),
                   [](math::hdr::Rgb<T_hdrChannel> aHdrPixel) -> math::sdr::Rgb
                   {
                        return to_sdr(aHdrPixel);
                   });
    return result;
}


//
// Explicit instantiations
//
template class Image<math::sdr::Rgb>;
template class Image<math::sdr::Rgba>;
template class Image<math::sdr::Grayscale>;

template class Image<math::hdr::Rgb_f>;

template Image<math::hdr::Rgb_f> to_hdr(const Image<math::sdr::Rgb> &);
template Image<math::sdr::Rgb> tonemap(const Image<math::hdr::Rgb_f> &);


} // namespace arte
} // namespace ad