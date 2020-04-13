#pragma once

#include <handy/ZeroOnMove.h>

#include <math/Color.h>

#include <filesystem>
#include <memory>


namespace ad {


// std::filesystem::path would be better, but requires macOS >= 10.15
using path = std::string;

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

    // \brief Low-level constructor, intended for loaders implementation, not general usage
    // \attention The calling code is responsible for providing a raster of appropriate size
    Image(math::Size<2, int> aDimensions, std::unique_ptr<char[]> aRaster);

    // \brief Creates an image
    Image(math::Size<2, int> aDimensions, pixel_format_t aBackgroundValue);

    // TODO Generalize for all supported formats
    void writePpm(std::ostream & aOut) const;
    void writePpm(std::ostream && aOut) const
    { return writePpm(aOut); };

    // TODO Generalize for all supported formats
    static Image ReadPpm(std::istream & aIn);

    static Image LoadFile(const path & aImageFile);

    Column operator[](std::size_t aColumnId);
    const_Column operator[](std::size_t aColumnId) const;

    // NOTE This is our firewall, behing this interface is encapsulated the old unsafe way
    pixel_format_t * data()
    { return reinterpret_cast<pixel_format_t *>(mRaster.get()); }

    const pixel_format_t * data() const
    { return reinterpret_cast<const pixel_format_t *>(mRaster.get()); }

    std::size_t size_bytes() const
    { return dimensions().area() * pixel_size_v; }

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

private:
    math::Size<2, ZeroOnMove<int>> mDimensions{0, 0};
    // NOTE: it is not possible to allocate an array of non-default constructible objects
    //std::unique_ptr<T_pixelFormat[]> mRaster{nullptr};
    // TODO try with byte
    std::unique_ptr<char[]> mRaster{nullptr};
};


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


} // namespace ad
