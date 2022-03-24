#include "LoadBuffer.h"

#include "Logging.h"

#include <handy/Base64.h>

#include <span>
#include <strstream>


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


std::vector<std::byte> loadDataUri(arte::gltf::Uri aUri)
{
    constexpr const char * base64Prefix{"base64,"};

    std::string_view encoded{aUri.string};
    encoded.remove_prefix(encoded.find(base64Prefix) + std::strlen(base64Prefix));
    return handy::base64::decode(encoded);
}


std::vector<std::byte> loadBufferData(arte::Const_Owned<arte::gltf::Buffer> aBuffer)
{
    if (!aBuffer->uri)
    {
        ADLOG(gPrepareLogger, critical)
             ("Buffer #{} does not have target defined.", aBuffer.id());
        throw std::logic_error{"Buffer was expected to have an Uri."};
    }

    auto uri = *aBuffer->uri;
    switch(uri.type)
    {
    case arte::gltf::Uri::Type::Data:
    {
        ADLOG(gPrepareLogger, trace)("Buffer #{} data is read from a data URI.", aBuffer.id());
        return loadDataUri(uri);
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


const std::map<arte::gltf::Image::MimeType, arte::ImageFormat> gMimeToFormat {
    {arte::gltf::Image::MimeType::ImageJpeg, arte::ImageFormat::Jpg},
    {arte::gltf::Image::MimeType::ImagePng, arte::ImageFormat::Png},
};



arte::Image<math::sdr::Rgba>
loadImageFromBytes(std::span<std::byte> aBytes, arte::gltf::Image::MimeType aMime)
{
    using Image = arte::Image<math::sdr::Rgba>;

    // TODO Ad 2022/03/23: Replace deprecated istrstream with a proposed alternative
    // see: https://en.cppreference.com/w/cpp/io/istrstream
    // see: https://stackoverflow.com/a/12646922/1027706
    return Image::Read(
        gMimeToFormat.at(aMime),
        std::istrstream(reinterpret_cast<const char *>(aBytes.data()), aBytes.size()),
        arte::ImageOrientation::InvertVerticalAxis);
}


arte::Image<math::sdr::Rgba>
loadImageData(arte::Const_Owned<arte::gltf::Image> aImage)
{
    using Image = arte::Image<math::sdr::Rgba>;

    if(const arte::gltf::Uri * uri = std::get_if<arte::gltf::Uri>(&aImage->dataSource))
    {
        switch(uri->type)
        {
        case arte::gltf::Uri::Type::Data:
        {
            ADLOG(gPrepareLogger, trace)("Image #{} data is read from a data URI.", aImage.id());
            // TODO handle this situation by reading the magic numbers
            if (!aImage->mimeType)
            {
                ADLOG(gPrepareLogger, critical)
                     ("Unsupported: Image #{} has a data URI but no mime type.", aImage.id());
                throw std::logic_error{"Image with data URI but no mime type."};
            }
            auto bytes = loadDataUri(*uri);
            return loadImageFromBytes(bytes, *aImage->mimeType);
        }
        case arte::gltf::Uri::Type::File:
        {
            ADLOG(gPrepareLogger, trace)("Image #{} data is read from a file URI.", aImage.id());
            return Image{aImage.getFilePath(*uri), arte::ImageOrientation::InvertVerticalAxis};
        }
        default:
            throw std::logic_error{"Invalid uri type."};
        }
    }
    else
    {
        ADLOG(gPrepareLogger, trace)("Image #{} data is read from a buffer view.", aImage.id());
        auto bufferView =
            aImage.get(std::get<arte::gltf::Index<arte::gltf::BufferView>>(aImage->dataSource));

        if (!aImage->mimeType)
        {
            ADLOG(gPrepareLogger, critical)
                 ("Invalid file: Image #{} has a buffer view but no mime type.", aImage.id());
            throw std::logic_error{"Image with buffer view but no mime type."};
        }

        auto bytes = loadBufferData(bufferView.get(&arte::gltf::BufferView::buffer));
        return loadImageFromBytes(std::span<std::byte>{bytes}.subspan(bufferView->byteOffset,
                                                                      bufferView->byteLength),
                                  *aImage->mimeType);
    }
}


} // namespace gltfviewer
} // namespace ad
