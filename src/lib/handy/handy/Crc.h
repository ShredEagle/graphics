#pragma once


#include <array>
#include <string>


// Some implementation relying on a table, which is quite compact.
// It is supposed to be a 64 bit ECMA-182 polynomial, but cross checking with a the go tests fails.
// see: http://andrewl.dreamhosters.com/filedump/crc64.cpp
// TODO Check alternative
// see: https://gitlab.inria.fr/lawall/linux-next/-/blob/1ce80e0fe98e72af4125f47c65b0bc01b60a7feb/lib/gen_crc64table.c

namespace ad {
namespace handy {


constexpr std::uint64_t gPolynomial64 = 0xC96C5795D7870F42;
constexpr std::size_t gTableSize = 256;
using Crc64 = std::uint64_t;
using Crc64Table = std::array<Crc64, gTableSize>;


namespace detail {

    constexpr Crc64Table makeTable()
    {
        Crc64Table result{0};

        for(std::size_t i = 0; i < gTableSize; ++i)
        {
            Crc64 crc{i};
            for(std::uint64_t j = 0; j < 8; ++j)
            {
                if (crc & 1)
                {
                    crc >>= 1;
                    crc ^= gPolynomial64;
                }
                else
                {
                    crc >>= 1;
                }
            }
            result[i] = crc;
        }

        return result;
    }

    static constexpr Crc64Table gTable{detail::makeTable()};

} // namespace detail


template <class T_iterator>
constexpr Crc64 crc64(T_iterator aBegin, T_iterator aEnd)
{
    // Sadly, this seems not to be allowed by Cpp
    // see: https://www.reddit.com/r/cpp/comments/b04smy/static_variable_not_permitted_in_a_constexpr/
    // Yet, it is content if the static constexpr lives outside the function.
    //static constexpr Crc64Table table{detail::makeTable()};

    Crc64 crc{0};
    for(; aBegin != aEnd; ++aBegin)
    {
        std::byte data = static_cast<std::byte>(*aBegin);
        std::size_t index = ((Crc64)data ^ crc) & 0xFF;
        crc >>= 8;
        crc ^= detail::gTable[index];
    }
    return crc;
}


inline constexpr Crc64 crc64(std::string_view aStringView)
{
    return crc64(aStringView.begin(), aStringView.end());
}


} // namespace handy
} // namespace ad
