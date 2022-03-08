#include "GltfRendering.h"

#include "Logging.h"
#include "Shaders.h"

#include <handy/Base64.h>

#include <renderer/GL_Loader.h>
#include <renderer/Shading.h>


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
std::vector<std::byte> loadBufferData(Const_Owned<gltf::BufferView> aBufferView)
{
    auto buffer = aBufferView.get(&gltf::BufferView::buffer);

    if (!buffer->uri)
    {
        ADLOG(gMainLogger, critical)
             ("Buffer #{} does not have target defined.", aBufferView->buffer);
        throw std::logic_error{"Buffer was expected to have an Uri."};
    }

    constexpr const char * base64Prefix{"base64,"};

    gltf::Uri uri = *buffer->uri;
    switch(uri.type)
    {
    case gltf::Uri::Type::Data:
    {
        std::string_view encoded{uri.string};
        encoded.remove_prefix(encoded.find(base64Prefix) + std::strlen(base64Prefix));
        return handy::base64::decode(encoded);
    }
    case gltf::Uri::Type::File:
        // TODO Implement buffer content from file
        throw std::logic_error{"File uri not implemented yet."};
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
        ADLOG(gMainLogger, critical)
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


    return {bufferView, std::move(buffer)};
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
    // TODO use "scoped bounds" helpers
    glBindVertexArray(vao);

    for (const auto & [semantic, accessorIndex] : aPrimitive.elem().attributes)
    {
        ADLOG(gMainLogger, trace)("Accessor {}: {}", semantic, accessorIndex);
        Const_Owned<gltf::Accessor> accessor = aPrimitive.get(accessorIndex);

        // All accessors for a given primitive must have the same count.
        count = accessor->count;

        if (!accessor->bufferView)
        {
            // TODO Handle no buffer view (accessor initialized to zeros)
            ADLOG(gMainLogger, error)
                 ("Unsupported: accessor #{} does not have a buffer view.", accessorIndex);
            continue;
        }

        // TODO 1 handle interleaved vertex buffers, instead of always making a new gl buffer.
        auto [bufferView, vertexBuffer] = prepareBuffer_impl<graphics::VertexBufferObject>(accessor);
        // From here on the corresponding vertex buffer is bound.

        vbos.push_back(std::move(vertexBuffer));

        if (auto found = gSemanticToAttribute.find(semantic);
            found != gSemanticToAttribute.end())
        {
            VertexAttributeLayout layout = gElementTypeToLayout.at(accessor->type);
            if (layout.occupiedAttributes != 1)
            {
                throw std::logic_error{"Matrix attributes not implemented yet."};
            }

            // The vertex attributes in the shader are float, so use glVertexAttribPointer.
            glVertexAttribPointer(found->second,
                                  layout.componentsPerAttribute,
                                  accessor->componentType,
                                  accessor->normalized,
                                  static_cast<GLsizei>(bufferView->byteStride ? *bufferView->byteStride : 0),
                                  // Note: The buffer view byte offset is directly taken into account when loading data with glBufferData().
                                  reinterpret_cast<void *>(accessor->byteOffset)
                                  );
            glEnableVertexAttribArray(found->second);
        }
    }

    if (aPrimitive->indices)
    {
        auto indicesAccessor = aPrimitive.get(&gltf::Primitive::indices);
        indices = Indices{indicesAccessor};
        count = indicesAccessor->count;
    }

    glBindVertexArray(0);
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
        const Indices & indices = *aMeshPrimitive.indices;
        glDrawElements(aMeshPrimitive.drawMode, 
                       aMeshPrimitive.count,
                       indices.componentType,
                       reinterpret_cast<void *>(indices.byteOffset));
    }
    else
    {
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
