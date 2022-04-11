#pragma once


#include "3rdparty/beast_base64.h"


namespace ad {
namespace handy {
namespace base64 {


inline std::vector<std::byte> decode(std::string_view aBase64Encoded)
{
    namespace bd64 = boost::beast::detail::base64;

    std::vector<std::byte> decoded{bd64::decoded_size(aBase64Encoded.size())};

    auto [bytesWritten, charactersRead] = 
        bd64::decode(decoded.data(), aBase64Encoded.data(), aBase64Encoded.size());

    decoded.resize(bytesWritten);
    return decoded;
}


} // namespace base64
} // namespace handy
} // namespace ad
