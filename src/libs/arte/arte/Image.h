#pragma once

#include <handy/ZeroOnMove.h>

#include <platform/Filesystem.h>

#include <math/Color.h>
#include <math/Rectangle.h>

#include <algorithm>
#include <map>
#include <memory>


namespace ad {
namespace arte {


enum class ImageFormat
{
    Pgm,
    Ppm,
    Bmp,
    Jpg,
    Png,
    Hdr,
};


enum class ImageOrientation
{
    Unchanged,
    InvertVerticalAxis,
};


struct FormatInfo
{
    std::string name;
    filesystem::path extension;
};


static const std::map<ImageFormat, FormatInfo> gImageFormatMap {
    {ImageFormat::Pgm, {"PGM", ".pgm"}},
    {ImageFormat::Ppm, {"PPM", ".ppm"}},
    {ImageFormat::Bmp, {"BMP", ".bmp"}},
    {ImageFormat::Jpg, {"JPG", ".jpg"}},
    {ImageFormat::Png, {"PNG", ".png"}},
    {ImageFormat::Hdr, {"HDR", ".hdr"}},
};


inline std::string to_string(ImageFormat aFormat)
{
    return gImageFormatMap.at(aFormat).name;
}


inline filesystem::path to_extension(ImageFormat aFormat)
{
    return gImageFormatMap.at(aFormat).extension;
}


inline ImageFormat from_extension(filesystem::path aExtension)
{
    for(auto pair : gImageFormatMap)
    {
        if (pair.second.extension == aExtension)
        {
            return pair.first;
        }
    }
    throw std::out_of_range("No ImageFormat for extension '" + aExtension.string() + "'");
}



template <class T_pixelFormat>
class Image
{
    // > Objects of trivially-copyable types that are not potentially-overlapping subobjects
    // > are the only C++ objects that may be safely copied with std::memcpy
    // > or serialized to/from binary files with std::ofstream::write()/std::ifstream::read().
    // see: https://en.cppreference.com/w/cpp/types/is_trivially_copyable
    static_assert(std::is_trivially_copyable<T_pixelFormat>::value,
                  "T_pixelFormat must be a trivally copyable pixel type.");

public:
    using pixel_format_t = T_pixelFormat;
    static constexpr std::size_t pixel_size_v = sizeof(pixel_format_t);

    // alias useful in generic programming situations
    using value_type = pixel_format_t;

private:
    /// \brief Should never be kept in client code, only intended as a temporary
    template <class T_image>
    class Column_base
    {
        friend class Image;

    public:
        auto & operator[](std::size_t aRowId)
        {
            return mImage.data()[aRowId*mImage.width() + mColumnId];
        }

    private:
        Column_base(T_image & aImage, std::size_t aColumnId) noexcept :
            mImage(aImage),
            mColumnId(aColumnId)
        {}

        // Disable copy and move operations
        Column_base(const Column_base &) = delete;
        Column_base & operator=(const Column_base &) = delete;
        Column_base(Column_base &&) = delete;
        Column_base & operator=(Column_base &&) = delete;

        T_image & mImage;
        std::size_t mColumnId;
    };

    using Column = Column_base<Image>;
    using const_Column = Column_base<const Image>;

public:
    Image() = default;

    // Not trivially copyable: need to deep copy the unique ptr
    Image(const Image & aRhs);
    Image & operator=(const Image & aRhs);

    Image(Image && aRhs) noexcept = default;
    Image & operator=(Image && aRhs) noexcept = default;

    /// \brief Low-level constructor, intended for loaders implementation, not general usage
    /// \attention The calling code is responsible for providing a raster of appropriate size
    Image(math::Size<2, int> aDimensions, std::unique_ptr<unsigned char[]> aRaster);

    /// \brief Creates an image
    Image(math::Size<2, int> aDimensions, pixel_format_t aBackgroundValue);

    /// \brief Load an Image from a file on disk.
    /// \note Similar functionality to LoadFile().
    explicit Image(const filesystem::path & aImageFile,
                   ImageOrientation aOrientation = ImageOrientation::Unchanged);


    /// \brief Return an image of requested dimensions, where the pixel memory contains garbage.
    ///
    /// Each pixel can be written, but reading it before it is first written is an undefined behaviour.
    static Image makeUninitialized(math::Size<2, int> aDimensions);

    void write(ImageFormat aFormat, std::ostream & aOut,
               ImageOrientation aOrientation = ImageOrientation::Unchanged) const;
    void write(ImageFormat aFormat, std::ostream && aOut,
               ImageOrientation aOrientation = ImageOrientation::Unchanged) const
    { return write(aFormat, aOut, aOrientation); };

    static Image Read(ImageFormat aFormat, std::istream & aIn,
                      ImageOrientation aOrientation = ImageOrientation::Unchanged);
    static Image Read(ImageFormat aFormat, std::istream && aIn,
                      ImageOrientation aOrientation = ImageOrientation::Unchanged)
    { return Read(aFormat, aIn, aOrientation); }

    static Image LoadFile(const filesystem::path & aImageFile,
                          ImageOrientation aOrientation = ImageOrientation::Unchanged);

    void saveFile(const filesystem::path & aDestination,
                  ImageOrientation aOrientation = ImageOrientation::Unchanged) const;

    void clear(T_pixelFormat aClearColor);

    // TODO Is it a good idea? It works with the usual image semantix, i.e. [x][y], yet
    //   it is the opposite than for matrices. Plus the implementation creates complexity.
    Column operator[](std::size_t aColumnId);
    const_Column operator[](std::size_t aColumnId) const;

    pixel_format_t & at(std::size_t aColumn, std::size_t aRow)
    { return data()[aRow*width() + aColumn]; }

    pixel_format_t at(std::size_t aColumn, std::size_t aRow) const
    { return data()[aRow*width() + aColumn]; }

    template <class T_integer>
    pixel_format_t & at(math::Position<2, T_integer> aPosition)
    { return at(aPosition.x(), aPosition.y()); }

    template <class T_integer>
    pixel_format_t at(math::Position<2, T_integer> aPosition) const
    { return at(aPosition.x(), aPosition.y()); }

    // NOTE This is our firewall, behing this interface is encapsulated the old unsafe way
    pixel_format_t * data()
    { return reinterpret_cast<pixel_format_t *>(mRaster.get()); }

    const pixel_format_t * data() const
    { return reinterpret_cast<const pixel_format_t *>(mRaster.get()); }

    explicit operator const unsigned char * () const
    { return mRaster.get(); }

    explicit operator const std::byte * () const
    { return reinterpret_cast<const std::byte *>(mRaster.get()); }

    std::size_t size_bytes() const
    { return dimensions().area() * pixel_size_v; }

    std::size_t size_bytes_line() const
    { return dimensions().width() * pixel_size_v; }

    auto begin()
    { return data(); }

    auto end()
    { return data() + mDimensions.area(); }

    auto cbegin() const
    { return data(); }

    auto cend() const
    { return data() + mDimensions.area(); }

    auto begin() const
    { return cbegin(); }

    auto end() const
    { return cend(); }

    int width() const
    { return mDimensions.width(); }

    int height() const
    { return mDimensions.height(); }

    math::Size<2, int> dimensions() const
    { return static_cast<math::Size<2, int>>(mDimensions); }

    /// \brief The alignment of consecutive rows. This is important for OpenGL, which uses 4 by default.
    /// \note Currently 1 in all the cases for us, even stb_image tightly packs rows.
    std::size_t rowAlignment() const
    { return 1; }

    //
    // Image edition
    //
    Image crop(const math::Rectangle<int> & aZone) const;

    template <class T_iterator>
    Image prepareArray(T_iterator aFirstPosition, T_iterator aLastPosition, math::Size<2, int> aDimension) const;

    /// \brief Paste a copy of `aSource` image into this, placing it at `aPastePosition`.
    Image & pasteFrom(const Image & aSource, math::Position<2, int> aPastePosition);

private:
    /// \return Position immediatly after the last writen element.
    T_pixelFormat * cropTo(T_pixelFormat * aDestination, const math::Rectangle<int> & aZone) const;

    math::Size<2, ZeroOnMove<int>> mDimensions{0, 0};
    // NOTE: it is not possible to allocate an array of non-default constructible objects
    //std::unique_ptr<T_pixelFormat[]> mRaster{nullptr};
    // TODO try with byte
    std::unique_ptr<unsigned char[]> mRaster{nullptr};
};


Image<math::sdr::Grayscale> toGrayscale(const Image<math::sdr::Rgb> & aSource);


template <class T_hdrChannel = float>
Image<math::hdr::Rgb<T_hdrChannel>> to_hdr(const Image<math::sdr::Rgb> & aSource);

template <class T_hdrChannel>
Image<math::sdr::Rgb> tonemap(const Image<math::hdr::Rgb<T_hdrChannel>> & aSource);

template<class T_pixelFormat>
Image<T_pixelFormat> & decodeSRGBToLinear(Image<T_pixelFormat> & aImage);


template <class T_pixelFormat, std::forward_iterator T_iterator, class T_proj = std::identity>
Image<T_pixelFormat> stackVertical(T_iterator aFirst, T_iterator aLast, T_proj proj = {});

template <class T_pixelFormat, std::ranges::input_range T_range, class T_proj = std::identity>
Image<T_pixelFormat> stackVertical(T_range && aRange, T_proj proj = {});


using ImageRgb = Image<math::sdr::Rgb>;
using ImageRgba = Image<math::sdr::Rgba>;


//
// Implementations
//

template <class T_pixelFormat>
auto Image<T_pixelFormat>::operator[](std::size_t aColumnId) -> Image::Column
{
    return Column{*this, aColumnId};
}


template <class T_pixelFormat>
auto Image<T_pixelFormat>::operator[](std::size_t aColumnId) const -> Image::const_Column
{
    return const_Column{*this, aColumnId};
}


template <class T_pixelFormat>
template <class T_iterator>
Image<T_pixelFormat> Image<T_pixelFormat>::prepareArray(T_iterator aFirstPosition,
                                                        T_iterator aLastPosition,
                                                        math::Size<2, int> aDimension) const
{
    math::Size<2, int> targetDimensions{
        aDimension.width(),
        aDimension.height() * (int)std::distance(aFirstPosition, aLastPosition)};

    auto target = std::make_unique<unsigned char[]>(targetDimensions.area() * pixel_size_v);

    T_pixelFormat * destination = reinterpret_cast<T_pixelFormat *>(target.get());
    for(; aFirstPosition != aLastPosition; ++aFirstPosition)
    {
        destination = cropTo(destination, {*aFirstPosition, aDimension});
    }

    return {targetDimensions, std::move(target)};
}


template <class T_pixelFormat, std::forward_iterator T_iterator, class T_proj>
Image<T_pixelFormat> stackVertical(T_iterator aFirst, T_iterator aLast, T_proj proj)
{
    math::Size<2, int> atlasResolution = math::Size<2, int>::Zero();
    std::ranges::for_each(
        aFirst, aLast,
        [&](const Image<T_pixelFormat> & aImage)
        {
            // TODO that would make a nice factorized function
            atlasResolution.width() = std::max(atlasResolution.width(), aImage.dimensions().width());
            atlasResolution.height() += aImage.dimensions().height();
        },
        proj);

    // TODO When used to assemble texture aliases,
    // this loop could be optimized by writing directly into a texture of the appropriate dimension.
    arte::Image<math::sdr::Rgba> result{atlasResolution, math::sdr::gTransparent};
    math::Vec<2, int> offset{0, 0};
    std::ranges::for_each(
        aFirst, aLast,
        [&](const Image<T_pixelFormat> & aImage)
        {
            result.pasteFrom(aImage, offset.as<math::Position>());
            offset.y() += aImage.dimensions().height();
        },
        proj);

    return result;
}


template <class T_pixelFormat, std::ranges::input_range T_range, class T_proj>
Image<T_pixelFormat> stackVertical(T_range && aRange, T_proj proj)
{
    return stackVertical<T_pixelFormat>(std::begin(aRange), std::end(aRange), std::move(proj));
}


} // namespace arte
} // namespace ad
