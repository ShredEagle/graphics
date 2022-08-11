#pragma once


#include "../../Image.h"

#include "../3rdparty/stb_image_include.h"

#include <cassert>


namespace ad {
namespace arte {
namespace detail {


template <class T_pixel>
struct stbi_traits;


template <>
struct stbi_traits<math::sdr::Rgb>
{
    static constexpr int channels = STBI_rgb;
    static constexpr auto loadFromCallbacks = &stbi_load_from_callbacks;
};


template <>
struct stbi_traits<math::sdr::Rgba>
{
    static constexpr int channels = STBI_rgb_alpha;
    static constexpr auto loadFromCallbacks = &stbi_load_from_callbacks;
};


template <class T_number>
struct stbi_traits<math::hdr::Rgb<T_number>>
{
    static constexpr int channels = STBI_rgb;
    // IMPORTANT: load**f**
    static constexpr auto loadFromCallbacks = &stbi_loadf_from_callbacks;
};


struct StbImageFormats
{
    inline static const stbi_io_callbacks streamCallbacks{
        // Read
        [](void *user,char *data,int size) -> int
        {
            std::istream * input = reinterpret_cast<std::istream *>(user);
            input->read(data, size);
            return static_cast<int>(input->gcount());
        },
        // Skip
        [](void *user,int n)
        {
            std::istream * input = reinterpret_cast<std::istream *>(user);
            input->seekg(n, std::ios_base::cur);
        },
        // Eof
        [](void *user) -> int
        {
            std::istream * input = reinterpret_cast<std::istream *>(user);
            return input->eof() ? 1 : 0;
        }
    };


    template <class T_pixel>
    static Image<T_pixel> Read(std::istream & aIn, ImageOrientation aOrientation)
    {
        if (aOrientation == ImageOrientation::InvertVerticalAxis)
        {
            stbi_set_flip_vertically_on_load(true);
        }

        math::Size<2, int> dimension = math::Size<2, int>::Zero();
        int channelsInFile;

        // stbi_traits will redirect to either `load` or `loadf` based on pixel type.
        unsigned char * data = (unsigned char *)stbi_traits<T_pixel>::loadFromCallbacks(
            &streamCallbacks,
            &aIn,
            &dimension.width(),
            &dimension.height(),
            &channelsInFile,
            stbi_traits<T_pixel>::channels);

        assert(channelsInFile >= 3);

        // HDR CASE
        // IMPORTANT: even though it returns a float*, stbi_loadf internally allocates
        // the returned data buffer as a char* via our provided STBI_MALLOC.
        // So the default Deleter is right.
        // TODO Ad 2022/06/08: I am nonetheless not sure whether it is safe to
        // then treat the returned array as an array of T_pixel (math::hdr::Rgb_f).
        // overview of the issue: https://stackoverflow.com/a/70157161/1027706
        return Image<T_pixel>{dimension, std::unique_ptr<unsigned char []>{data}};
    };
};


} // namespace detail
} // namespace arte
} // namespace ad
