#pragma once


#include "../3rdparty/stb_image_include.h"


namespace ad {
namespace arte {
namespace detail {


template <class T_pixel>
struct stbi_channels;


template <>
struct stbi_channels<math::sdr::Rgb>
{
    static constexpr int value = STBI_rgb;
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
    static Image<T_pixel> Read(std::istream & aIn)
    {
        //  TODO
        //stbi_set_flip_vertically_on_load(true);

        math::Size<2, int> dimension = math::Size<2, int>::Zero();
        int channelsInFile;

        unsigned char * data = stbi_load_from_callbacks(
            &streamCallbacks,
            &aIn,
            &dimension.width(),
            &dimension.height(),
            &channelsInFile,
            stbi_channels<T_pixel>::value);

        assert(channelsInFile >= 3);

        return Image<T_pixel>{dimension, std::unique_ptr<unsigned char []>{data}};
    };
};


} // namespace detail
} // namespace arte
} // namespace ad
