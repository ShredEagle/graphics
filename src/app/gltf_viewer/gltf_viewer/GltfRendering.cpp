#include "GltfRendering.h"

#include "Logging.h"
#include "Shaders.h"

#include <handy/Base64.h>

#include <renderer/GL_Loader.h>
#include <renderer/Shading.h>

#include <span>


namespace ad {

using namespace arte;

namespace gltfviewer {


/// \brief Maps semantic to vertex attribute indices that will be used in shaders.
const std::map<std::string /*semantic*/, GLuint /*vertex attribute index*/> gSemanticToAttribute{
    {"POSITION", 0},
    {"NORMAL", 1},
};


struct VertexAttributeLayout
{
    GLint componentsPerAttribute;
    std::size_t occupiedAttributes{1}; // For matrices types, will be the number of columns.

    std::size_t totalComponents() const
    { return componentsPerAttribute * occupiedAttributes; }
};

using ElementType = gltf::Accessor::ElementType;

const std::map<gltf::Accessor::ElementType, VertexAttributeLayout> gElementTypeToLayout{
    {ElementType::Scalar, {1}},    
    {ElementType::Vec2,   {2}},    
    {ElementType::Vec3,   {3}},    
    {ElementType::Vec4,   {4}},    
    {ElementType::Mat2,   {2, 2}},    
    {ElementType::Mat3,   {3, 3}},    
    {ElementType::Mat4,   {4, 4}},    
};


//
// Helper functions
//

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


std::vector<std::byte> loadBufferData(Const_Owned<gltf::BufferView> aBufferView)
{
    auto buffer = aBufferView.get(&gltf::BufferView::buffer);

    if (!buffer->uri)
    {
        ADLOG(gPrepareLogger, critical)
             ("Buffer #{} does not have target defined.", aBufferView->buffer);
        throw std::logic_error{"Buffer was expected to have an Uri."};
    }

    constexpr const char * base64Prefix{"base64,"};

    gltf::Uri uri = *buffer->uri;
    switch(uri.type)
    {
    case gltf::Uri::Type::Data:
    {
        ADLOG(gPrepareLogger, trace)("Buffer #{} data is read from a data URI.", aBufferView->buffer);
        std::string_view encoded{uri.string};
        encoded.remove_prefix(encoded.find(base64Prefix) + std::strlen(base64Prefix));
        return handy::base64::decode(encoded);
    }
    case gltf::Uri::Type::File:
    {
        ADLOG(gPrepareLogger, trace)("Buffer #{} data is read from file {}.", aBufferView->buffer, uri.string);
        return loadInputStream(
            std::ifstream{buffer.getFilePath(&gltf::Buffer::uri).string(),
                          std::ios_base::in | std::ios_base::binary},
            buffer->byteLength, 
            uri.string);
    }
    default:
        throw std::logic_error{"Invalid uri type."};
    }
}


template <class T_buffer>
std::pair<Const_Owned<gltf::BufferView>, T_buffer>
prepareBuffer_impl(Const_Owned<gltf::Accessor> aAccessor)
{
    Const_Owned<gltf::BufferView> bufferView = aAccessor.get(&gltf::Accessor::bufferView);

    T_buffer buffer;

    if (!bufferView->target)
    {
        ADLOG(gPrepareLogger, critical)
             ("Buffer view #{} does not have target defined.", *aAccessor->bufferView);
        throw std::logic_error{"Buffer view was expected to have a target."};
    }

    glBindBuffer(*bufferView->target, buffer);
    glBufferData(*bufferView->target,
                 bufferView->byteLength,
                 // TODO might be even better to only load in main memory the part of the buffer starting
                 // at bufferView->byteOffset (and also limit the length there, actually).
                 loadBufferData(bufferView).data() + bufferView->byteOffset,
                 GL_STATIC_DRAW);

    ADLOG(gPrepareLogger, debug)
         ("Loaded {} bytes in target {}, offset in source buffer is {} bytes.",
          bufferView->byteLength,
          *bufferView->target,
          bufferView->byteOffset);

    return {bufferView, std::move(buffer)};
}


template <class T_component>
void outputElements(std::ostream & aOut, std::span<T_component> aData, std::size_t aElementCount, VertexAttributeLayout aLayout)
{
    std::size_t accessId = 0;
    for (std::size_t elementId = 0; elementId != aElementCount; ++elementId)
    {
        aOut << "{";
        // All element types have at least 1 component.
        aOut << aData[accessId++];
        for (std::size_t componentId = 1; componentId != aLayout.totalComponents(); ++componentId)
        {
            aOut << ", " << aData[accessId];
            ++accessId;
        }
        aOut << "}, ";
    }
}


template <class T_component>
void analyze_impl(Const_Owned<gltf::Accessor> aAccessor,
                  Const_Owned<gltf::BufferView> aBufferView,
                  const std::vector<std::byte> & aBytes)
{
    VertexAttributeLayout layout = gElementTypeToLayout.at(aAccessor->type);

    std::span<const T_component> span{
        reinterpret_cast<const T_component *>(aBytes.data() + aBufferView->byteOffset + aAccessor->byteOffset), 
        aAccessor->count * layout.totalComponents()
    };

    std::ostringstream oss;
    outputElements(oss, span, aAccessor->count, layout);
    ADLOG(gPrepareLogger, debug)("Accessor content:\n{}", oss.str());
}


void analyzeAccessor(Const_Owned<gltf::Accessor> aAccessor)
{
    Const_Owned<gltf::BufferView> bufferView = aAccessor.get(&gltf::Accessor::bufferView);

    if (bufferView->byteStride)
    {
        ADLOG(gPrepareLogger, critical)
             ("Accessor's buffer view #{} has a byte stride, which is not currently supported.", *aAccessor->bufferView);
        throw std::logic_error{"Byte stride not supported in analyze."};
    }

    std::vector<std::byte> bytes = loadBufferData(bufferView);

    switch(aAccessor->componentType)
    {
    default:
        ADLOG(gPrepareLogger, error)
             ("Analysis not available for component type {}.", aAccessor->componentType);
        return;
    case GL_UNSIGNED_SHORT:
    {
        analyze_impl<GLshort>(aAccessor, bufferView, bytes);
        break;
    }
    case GL_FLOAT:
    {
        analyze_impl<GLfloat>(aAccessor, bufferView, bytes);
        break;
    }
    }
}

//
// Loaded buffers types
//
Indices::Indices(Const_Owned<gltf::Accessor> aAccessor) :
    componentType{aAccessor->componentType},
    byteOffset{aAccessor->byteOffset}
{
    auto [bufferView, indexBuffer] = prepareBuffer_impl<graphics::IndexBufferObject>(aAccessor);
    ibo = std::move(indexBuffer);
}


MeshPrimitive::MeshPrimitive(Const_Owned<gltf::Primitive> aPrimitive) :
    drawMode{aPrimitive->mode}
{
    graphics::bind_guard boundVao{vao};

    for (const auto & [semantic, accessorIndex] : aPrimitive.elem().attributes)
    {
        ADLOG(gPrepareLogger, debug)("Semantic '{}' is associated to accessor #{}", semantic, accessorIndex);
        Const_Owned<gltf::Accessor> accessor = aPrimitive.get(accessorIndex);

        // All accessors for a given primitive must have the same count.
        count = accessor->count;

        if (!accessor->bufferView)
        {
            // TODO Handle no buffer view (accessor initialized to zeros)
            ADLOG(gPrepareLogger, error)
                 ("Unsupported: accessor #{} does not have a buffer view.", accessorIndex);
            continue;
        }

        // TODO 1 handle interleaved vertex buffers, instead of always making a new gl buffer.
        auto [bufferView, vertexBuffer] = prepareBuffer_impl<graphics::VertexBufferObject>(accessor);
        // From here on the corresponding vertex buffer is bound.

        analyzeAccessor(accessor);

        vbos.push_back(std::move(vertexBuffer));

        if (auto found = gSemanticToAttribute.find(semantic);
            found != gSemanticToAttribute.end())
        {
            VertexAttributeLayout layout = gElementTypeToLayout.at(accessor->type);
            if (layout.occupiedAttributes != 1)
            {
                throw std::logic_error{"Matrix attributes not implemented yet."};
            }

            glEnableVertexAttribArray(found->second);


            GLsizei stride = static_cast<GLsizei>(bufferView->byteStride ? *bufferView->byteStride : 0);
            // The vertex attributes in the shader are float, so use glVertexAttribPointer.
            glVertexAttribPointer(found->second,
                                  layout.componentsPerAttribute,
                                  accessor->componentType,
                                  accessor->normalized,
                                  stride,
                                  // Note: The buffer view byte offset is directly taken into account when loading data with glBufferData().
                                  reinterpret_cast<void *>(accessor->byteOffset)
                                  );

            ADLOG(gPrepareLogger, debug)
                 ("Attached semantic '{}' to vertex attribute {}. Source data elements have {} components of type {}. Stride is {}, offset is {}.",
                  semantic, found->second,
                  layout.componentsPerAttribute, accessor->componentType,
                  stride, accessor->byteOffset);
        }
        else
        {
            ADLOG(gPrepareLogger, warn)("Semantic '{}' is ignored.", semantic);
        }
    }

    if (aPrimitive->indices)
    {
        auto indicesAccessor = aPrimitive.get(&gltf::Primitive::indices);
        indices = Indices{indicesAccessor};
        count = indicesAccessor->count;

        analyzeAccessor(indicesAccessor);
    }
}


Mesh prepare(arte::Const_Owned<arte::gltf::Mesh> aMesh)
{
    Mesh mesh;
    for (auto & primitive : aMesh.iterate(&arte::gltf::Mesh::primitives))     
    {
        mesh.primitives.emplace_back(primitive);
    }
    return mesh;
}


void render(const MeshPrimitive & aMeshPrimitive)
{
    graphics::bind_guard boundVao{aMeshPrimitive.vao};
    if (aMeshPrimitive.indices)
    {
        ADLOG(gDrawLogger, trace)
             ("Indexed rendering of {} vertices with mode {}.", aMeshPrimitive.count, aMeshPrimitive.drawMode);

        const Indices & indices = *aMeshPrimitive.indices;
        glDrawElements(aMeshPrimitive.drawMode, 
                       aMeshPrimitive.count,
                       indices.componentType,
                       reinterpret_cast<void *>(indices.byteOffset));
    }
    else
    {
        ADLOG(gDrawLogger, trace)
             ("Array rendering of {} vertices with mode {}.", aMeshPrimitive.count, aMeshPrimitive.drawMode);

        glDrawArrays(aMeshPrimitive.drawMode,  
                     0, // Start at the beginning of enable arrays, all byte offsets are aleady applied.
                     aMeshPrimitive.count);
    }
}


void render(const Mesh & aMesh)
{
    graphics::Program program = graphics::makeLinkedProgram({
        {GL_VERTEX_SHADER,   gltfviewer::gNaiveVertexShader},
        {GL_FRAGMENT_SHADER, gltfviewer::gNaiveFragmentShader},
    });

    graphics::bind_guard boundProgram{program};

    for (const auto & primitive : aMesh.primitives)
    {
        render(primitive);
    }
}


} // namespace gltfviewer
} // namespace ad
