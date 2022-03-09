#include "GltfRendering.h"

#include "Logging.h"
#include "Shaders.h"

#include <renderer/Uniforms.h>

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


// TODO accept the buffer view directly, but it require to embed the index in it for debug print.
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

    // Sanity check, that the target in the buffer view matches the type of GL buffer.
    if constexpr(std::is_same_v<T_buffer, graphics::VertexBufferObject>)
    {
        assert(*bufferView->target == GL_ARRAY_BUFFER);
    }
    else if constexpr(std::is_same_v<T_buffer, graphics::IndexBufferObject>)
    {
        assert(*bufferView->target == GL_ELEMENT_ARRAY_BUFFER);
    }

    glBindBuffer(*bufferView->target, buffer);
    glBufferData(*bufferView->target,
                 bufferView->byteLength,
                 // TODO might be even better to only load in main memory the part of the buffer starting
                 // at bufferView->byteOffset (and also limit the length there, actually).
                 loadBufferData(bufferView).data() + bufferView->byteOffset,
                 GL_STATIC_DRAW);
    glBindBuffer(*bufferView->target, 0);

    ADLOG(gPrepareLogger, debug)
         ("Loaded {} bytes in target {}, offset in source buffer is {} bytes.",
          bufferView->byteLength,
          *bufferView->target,
          bufferView->byteOffset);

    return {bufferView, std::move(buffer)};
}


template <class T_component>
void outputElements(std::ostream & aOut,
                    std::span<T_component> aData,
                    std::size_t aElementCount,
                    VertexAttributeLayout aLayout,
                    std::size_t aComponentStride)
{
    std::size_t accessId = 0;
    for (std::size_t elementId = 0; elementId != aElementCount; ++elementId)
    {
        aOut << "{";
        // All element types have at least 1 component.
        aOut << aData[accessId];
        for (std::size_t componentId = 1; componentId != aLayout.totalComponents(); ++componentId)
        {
            aOut << ", " << aData[accessId + componentId];
        }
        aOut << "}, ";

        accessId += aComponentStride;
    }
}


template <class T_component>
void analyze_impl(Const_Owned<gltf::Accessor> aAccessor,
                  Const_Owned<gltf::BufferView> aBufferView,
                  const std::vector<std::byte> & aBytes)
{
    VertexAttributeLayout layout = gElementTypeToLayout.at(aAccessor->type);

    // If there is no explicit stride, the vertex attribute elements are tightly packed
    // i.e. the stride, in term of components, is the number of components in one element.
    std::size_t componentStride = layout.totalComponents();
    if (aBufferView->byteStride)
    {
        auto stride = *aBufferView->byteStride;
        assert(stride % sizeof(T_component) == 0);
        componentStride = stride / sizeof(T_component);
    }

    std::span<const T_component> span{
        reinterpret_cast<const T_component *>(aBytes.data() + aBufferView->byteOffset + aAccessor->byteOffset), 
        // All the components, but not more (i.e. no "stride padding" after the last component)
        componentStride * (aAccessor->count - 1) + layout.totalComponents()
    };

    std::ostringstream oss;
    outputElements(oss, span, aAccessor->count, layout, componentStride);
    ADLOG(gPrepareLogger, debug)("Accessor content:\n{}", oss.str());
}


void analyzeAccessor(Const_Owned<gltf::Accessor> aAccessor)
{
    Const_Owned<gltf::BufferView> bufferView = aAccessor.get(&gltf::Accessor::bufferView);

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


// TODO accept the buffer view directly, but it require to embed the index in it for debug print.
const ViewerVertexBuffer & MeshPrimitive::prepareVertexBuffer(Const_Owned<gltf::Accessor> aAccessor)
{
    assert(aAccessor->bufferView);

    auto found = vbos.find(*aAccessor->bufferView);
    if (found == vbos.end())
    {
        auto [bufferView, vertexBuffer] = 
            prepareBuffer_impl<graphics::VertexBufferObject>(aAccessor);
        auto inserted = 
            vbos.emplace(*aAccessor->bufferView,
                         ViewerVertexBuffer{
                            std::move(vertexBuffer), 
                            bufferView->byteStride ? (GLsizei)*bufferView->byteStride : 0});
        return inserted.first->second;
    }
    else
    {
        return found->second;
    }
}


MeshPrimitive::MeshPrimitive(Const_Owned<gltf::Primitive> aPrimitive) :
    drawMode{aPrimitive->mode}
{
    graphics::bind_guard boundVao{vao};

    for (const auto & [semantic, accessorIndex] : aPrimitive->attributes)
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

        const ViewerVertexBuffer & vertexBuffer = prepareVertexBuffer(accessor);

        analyzeAccessor(accessor);

        if (auto found = gSemanticToAttribute.find(semantic);
            found != gSemanticToAttribute.end())
        {
            VertexAttributeLayout layout = gElementTypeToLayout.at(accessor->type);
            if (layout.occupiedAttributes != 1)
            {
                throw std::logic_error{"Matrix attributes not implemented yet."};
            }

            glEnableVertexAttribArray(found->second);

            graphics::bind_guard boundBuffer{vertexBuffer.vbo};

            // The vertex attributes in the shader are float, so use glVertexAttribPointer.
            glVertexAttribPointer(found->second,
                                  layout.componentsPerAttribute,
                                  accessor->componentType,
                                  accessor->normalized,
                                  vertexBuffer.stride,
                                  // Note: The buffer view byte offset is directly taken into account when loading data with glBufferData().
                                  reinterpret_cast<void *>(accessor->byteOffset)
                                  );

            ADLOG(gPrepareLogger, debug)
                 ("Attached semantic '{}' to vertex attribute {}."
                  " Source data elements have {} components of type {}."
                  " OpenGL buffer #{}, stride is {}, offset is {}.",
                  semantic, found->second,
                  layout.componentsPerAttribute, accessor->componentType,
                  vertexBuffer.vbo, vertexBuffer.stride, accessor->byteOffset);
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

std::ostream & operator<<(std::ostream & aOut, const MeshPrimitive & aPrimitive)
{
    return aOut << "<gltfviewer::MeshPrimitive> " 
                << (aPrimitive.indices ? "indexed" : "non-indexed")
                << " with " << aPrimitive.vbos.size() << " vbos."
        ;
}


std::ostream & operator<<(std::ostream & aOut, const Mesh & aMesh)
{
    aOut << "<gltfviewer::Mesh> " 
         << " with " << aMesh.primitives.size() << " primitives:"
        ;

    for (const auto & primitive : aMesh.primitives)
    {
        aOut << "\n\t* " << primitive;
    }
    
    return aOut;
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
        graphics::bind_guard boundIndexBuffer{indices.ibo};
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


Renderer::Renderer() :
    mProgram{graphics::makeLinkedProgram({
        {GL_VERTEX_SHADER,   gltfviewer::gNaiveVertexShader},
        {GL_FRAGMENT_SHADER, gltfviewer::gNaiveFragmentShader},
    })}
{}


void Renderer::render(const Mesh & aMesh) const
{
    graphics::bind_guard boundProgram{mProgram};

    for (const auto & primitive : aMesh.primitives)
    {
        gltfviewer::render(primitive);
    }
}


void Renderer::setCameraTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation)
{
    setUniform(mProgram, "u_camera", aTransformation); 
}


void Renderer::setProjectionTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation)
{
    setUniform(mProgram, "u_projection", aTransformation); 
}


} // namespace gltfviewer
} // namespace ad
