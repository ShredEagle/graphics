#include "LoadBuffer.h"

#include "DataLayout.h"
#include "Logging.h"
#include "Url.h"

#include <handy/Base64.h>

#include <span>
#include <strstream>

#include <renderer/GL_Loader.h>


namespace ad {
namespace gltfviewer {


arte::Const_Owned<arte::gltf::BufferView> 
checkedBufferView(arte::Const_Owned<arte::gltf::Accessor> aAccessor)
{
    if (!aAccessor->bufferView)
    {
        ADLOG(gPrepareLogger, critical)
             ("Unsupported: Accessor #{} does not have a buffer view associated.", aAccessor.id());
        throw std::logic_error{"Accessor was expected to have a buffer view."};
    }
    return aAccessor.get(&arte::gltf::Accessor::bufferView);
}


std::vector<std::byte> loadInputStream(std::istream && aInput, std::size_t aByteLength, const std::string & aStreamId)
{
    constexpr std::size_t gChunkSize = 128 * 1024;

    std::vector<std::byte> result(aByteLength);
    std::size_t remainingBytes = aByteLength;

    char * destination = reinterpret_cast<char *>(result.data());
    while (remainingBytes)
    {
        std::size_t readSize = std::min(remainingBytes, gChunkSize);
        if (!aInput.read(destination, readSize).good())
        {
            throw std::runtime_error{"Problem reading '" + aStreamId + "': "
                // If stream is not good(), yet converts to 'true', it means only eofbit is set.
                + (aInput ? "stream truncacted." : "read error.")};
        }
        destination += readSize;
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
            std::ifstream{decodeUrl(aBuffer.getFilePath(&arte::gltf::Buffer::uri).string()),
                          std::ios_base::in | std::ios_base::binary},
            aBuffer->byteLength, 
            uri.string);
    }
    default:
        throw std::logic_error{"Invalid uri type."};
    }
}


// Note: not part of the public interface for the moment
// I am afraid users will be confused whether the buffer view offset is applied.
std::vector<std::byte> loadBufferData(arte::Const_Owned<arte::gltf::BufferView> aBufferView)
{
    return loadBufferData(aBufferView.get(&arte::gltf::BufferView::buffer));
}


template <class T_component>
std::vector<GLuint> copyIndices(const std::byte * aFirst, std::size_t aCount)
{
    const T_component * first = reinterpret_cast<const T_component *>(aFirst);
    std::vector<GLuint> result;
    std::copy(first, first + aCount, std::back_inserter(result));
    return result;
}

/// \brief Converts all potential indice types to the largest representation.
std::vector<GLuint>
loadIndices(arte::Const_Owned<arte::gltf::accessor::Indices> aIndices, std::size_t aCount)
{
    auto bufferView = aIndices.get(&arte::gltf::accessor::Indices::bufferView);
    std::vector<std::byte> buffer = loadBufferData(bufferView);

    std::byte * first = buffer.data() + bufferView->byteOffset + aIndices->byteOffset;
    switch(aIndices->componentType)
    {
    case GL_UNSIGNED_BYTE:
        return copyIndices<GLubyte>(first, aCount);
    case GL_UNSIGNED_SHORT:
        return copyIndices<GLushort>(first, aCount);
    case GL_UNSIGNED_INT:
        return copyIndices<GLuint>(first, aCount);
    }

    throw std::logic_error{std::string{"In "} + __func__ 
        + ", unhandled component type: " + std::to_string(aIndices->componentType)};
}


std::vector<std::byte> 
loadBufferData(arte::Const_Owned<arte::gltf::Accessor> aAccessor)
{
    auto dataBufferView = checkedBufferView(aAccessor);
    std::vector<std::byte> bufferData = loadBufferData(dataBufferView);

    if(aAccessor->sparse)
    {
        auto sparse = aAccessor.get(&arte::gltf::Accessor::sparse);
        std::vector<GLuint> indices =
            loadIndices(sparse.get(&arte::gltf::accessor::Sparse::indices), sparse->count);

        auto values = sparse.get(&arte::gltf::accessor::Sparse::values);
        auto valuesBufferView = values.get(&arte::gltf::accessor::Values::bufferView);
        std::vector<std::byte> differenceBuffer = loadBufferData(valuesBufferView);
        const std::byte * difference = 
            differenceBuffer.data() + valuesBufferView->byteOffset + values->byteOffset;

        std::byte * element = 
            bufferData.data() + dataBufferView->byteOffset + aAccessor->byteOffset;
        const std::size_t elementSize = 
            gElementTypeToLayout.at(aAccessor->type).byteSize(aAccessor->componentType);

        std::size_t iteration = 0;
        for (auto modifiedIndex : indices)
        {
            std::copy(difference + iteration * elementSize,
                      difference + (iteration + 1) * elementSize,  
                      element + modifiedIndex * elementSize);
            ++iteration;
        }
    }

    return bufferData;
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
        arte::ImageOrientation::Unchanged);
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
            return Image{decodeUrl(aImage.getFilePath(*uri).string()), arte::ImageOrientation::Unchanged};
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
