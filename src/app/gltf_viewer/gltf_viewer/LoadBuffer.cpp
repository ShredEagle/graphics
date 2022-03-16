#include "LoadBuffer.h"

#include "Logging.h"

#include <handy/Base64.h>


namespace ad {
namespace gltfviewer {


std::vector<std::byte> loadInputStream(std::istream && aInput, std::size_t aByteLength, const std::string & aStreamId)
{
    constexpr std::size_t gChunkSize = 128 * 1024;

    std::vector<std::byte> result(aByteLength);
    std::size_t remainingBytes = aByteLength;
    
    while (remainingBytes)
    {
        std::size_t readSize = std::min(remainingBytes, gChunkSize);
        if (!aInput.read(reinterpret_cast<char *>(result.data()), readSize).good())
        {
            throw std::runtime_error{"Problem reading '" + aStreamId + "': "
                // If stream is not good(), yet converts to 'true', it means only eofbit is set.
                + (aInput ? "stream truncacted." : "read error.")};
        }
        remainingBytes -= readSize;
    }
    return result;
}


std::vector<std::byte> loadBufferData(arte::Const_Owned<arte::gltf::Buffer> aBuffer)
{
    if (!aBuffer->uri)
    {
        ADLOG(gPrepareLogger, critical)
             ("Buffer #{} does not have target defined.", aBuffer.id());
        throw std::logic_error{"Buffer was expected to have an Uri."};
    }

    constexpr const char * base64Prefix{"base64,"};

    arte::gltf::Uri uri = *aBuffer->uri;
    switch(uri.type)
    {
    case arte::gltf::Uri::Type::Data:
    {
        ADLOG(gPrepareLogger, trace)("Buffer #{} data is read from a data URI.", aBuffer.id());
        std::string_view encoded{uri.string};
        encoded.remove_prefix(encoded.find(base64Prefix) + std::strlen(base64Prefix));
        return handy::base64::decode(encoded);
    }
    case arte::gltf::Uri::Type::File:
    {
        ADLOG(gPrepareLogger, trace)("Buffer #{} data is read from file {}.", aBuffer.id(), uri.string);
        return loadInputStream(
            std::ifstream{aBuffer.getFilePath(&arte::gltf::Buffer::uri).string(),
                          std::ios_base::in | std::ios_base::binary},
            aBuffer->byteLength, 
            uri.string);
    }
    default:
        throw std::logic_error{"Invalid uri type."};
    }
}


} // namespace gltfviewer
} // namespace ad
