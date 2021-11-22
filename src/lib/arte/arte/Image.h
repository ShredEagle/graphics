#pragma once

#include <handy/ZeroOnMove.h>

#include <platform/Filesystem.h>

#include <math/Color.h>

#include <map>
#include <memory>


namespace ad {
namespace arte {


enum class ImageFormat
{
    Pgm,
    Ppm,
    Png,
    Bmp,
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
    {ImageFormat::Png, {"PNG", ".png"}},
    {ImageFormat::Bmp, {"BMP", ".bmp"}},
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



template <class T_pixelFormat = math::sdr::Rgb>
class Image
{
    static_assert(std::is_trivially_copyable<T_pixelFormat>::value,
                  "T_pixelFormat must be a trivally copyable pixel type.");

public:
    using pixel_format_t = T_pixelFormat;
    static constexpr std::size_t pixel_size_v = sizeof(pixel_format_t);

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

    /// \The alignment of consecutive rows. This is important for OpenGL, which uses 4 by default.
    /// \note Currently 1 in all the cases for us, even stb_image tightly packs rows.
    std::size_t rowAlignment() const
    { return 1; }

private:
    math::Size<2, ZeroOnMove<int>> mDimensions{0, 0};
    // NOTE: it is not possible to allocate an array of non-default constructible objects
    //std::unique_ptr<T_pixelFormat[]> mRaster{nullptr};
    // TODO try with byte
    std::unique_ptr<unsigned char[]> mRaster{nullptr};
};


Image<math::sdr::Grayscale> toGrayscale(const Image<math::sdr::Rgb> & aSource);


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


} // namespace arte
} // namespace ad
