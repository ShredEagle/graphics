#include "GltfRendering.h"

#include "LoadBuffer.h"
#include "Logging.h"
#include "Shaders.h"

#include <renderer/Uniforms.h>

#include <math/Transformations.h>

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


/// \brief First attribute index available for per-instance attributes.
constexpr GLuint gInstanceAttributeIndex = 8;

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
/// \brief Returns the buffer view associated to the accessor, or throw if there is none.
Const_Owned<gltf::BufferView> checkedBufferView(Const_Owned<gltf::Accessor> aAccessor)
{
    if (!aAccessor->bufferView)
    {
        ADLOG(gPrepareLogger, critical)
             ("Unsupported: Accessor #{} does not have a buffer view associated.", aAccessor.id());
        throw std::logic_error{"Accessor was expected to have a buffer view."};
    }
    return aAccessor.get(&gltf::Accessor::bufferView);
}


template <class T_buffer>
T_buffer prepareBuffer_impl(Const_Owned<gltf::BufferView> aBufferView)
{
    T_buffer buffer;

    if (!aBufferView->target)
    {
        ADLOG(gPrepareLogger, critical)
             ("Buffer view #{} does not have target defined.", aBufferView.id());
        throw std::logic_error{"Buffer view was expected to have a target."};
    }

    // Sanity check, that the target in the buffer view matches the type of GL buffer.
    if constexpr(std::is_same_v<T_buffer, graphics::VertexBufferObject>)
    {
        assert(*aBufferView->target == GL_ARRAY_BUFFER);
    }
    else if constexpr(std::is_same_v<T_buffer, graphics::IndexBufferObject>)
    {
        assert(*aBufferView->target == GL_ELEMENT_ARRAY_BUFFER);
    }

    glBindBuffer(*aBufferView->target, buffer);
    glBufferData(*aBufferView->target,
                 aBufferView->byteLength,
                 // TODO might be even better to only load in main memory the part of the buffer starting
                 // at aBufferView->byteOffset (and also limit the length there, actually).
                 loadBufferData(aBufferView.get(&gltf::BufferView::buffer)).data() 
                    + aBufferView->byteOffset,
                 GL_STATIC_DRAW);
    glBindBuffer(*aBufferView->target, 0);

    ADLOG(gPrepareLogger, debug)
         ("Loaded {} bytes in target {}, offset in source buffer is {} bytes.",
          aBufferView->byteLength,
          *aBufferView->target,
          aBufferView->byteOffset);

    return buffer;
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
    auto bufferView = checkedBufferView(aAccessor);

    std::vector<std::byte> bytes = loadBufferData(bufferView.get(&gltf::BufferView::buffer));

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
    byteOffset{aAccessor->byteOffset},
    ibo{prepareBuffer_impl<graphics::IndexBufferObject>(checkedBufferView(aAccessor))}
{}


const ViewerVertexBuffer & MeshPrimitive::prepareVertexBuffer(Const_Owned<gltf::BufferView> aBufferView)
{
    auto found = vbos.find(aBufferView.id());
    if (found == vbos.end())
    {
        auto vertexBuffer = prepareBuffer_impl<graphics::VertexBufferObject>(aBufferView);
        auto inserted = 
            vbos.emplace(aBufferView.id(),
                         ViewerVertexBuffer{
                            std::move(vertexBuffer), 
                            aBufferView->byteStride ? (GLsizei)*aBufferView->byteStride : 0});
        return inserted.first->second;
    }
    else
    {
        return found->second;
    }
}


void InstanceList::update(std::span<Instance> aInstances)
{
    {
        graphics::bind_guard boundBuffer{mVbo};
        // Orphan the previous buffer, if any
        glBufferData(GL_ARRAY_BUFFER, aInstances.size_bytes(), NULL, GL_STREAM_DRAW);
        // Copy value to new buffer
        glBufferSubData(GL_ARRAY_BUFFER, 0, aInstances.size_bytes(), aInstances.data());
    }
    mInstanceCount = aInstances.size();
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

        const ViewerVertexBuffer & vertexBuffer = prepareVertexBuffer(checkedBufferView(accessor));

        // TODO Have control whether analysis takes place
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

        // TODO Have control whether analysis takes place
        analyzeAccessor(indicesAccessor);
    }
}


MeshPrimitive::MeshPrimitive(arte::Const_Owned<arte::gltf::Primitive> aPrimitive,
                             const InstanceList & aInstances) :
    MeshPrimitive{aPrimitive}
{
    associateInstanceBuffer(aInstances);
}


void MeshPrimitive::associateInstanceBuffer(const InstanceList & aInstances)
{
    graphics::bind_guard boundVao{vao};
    graphics::bind_guard boundBuffer{aInstances.mVbo};

    for(GLuint attributeOffset = 0; attributeOffset != 4; ++attributeOffset)
    {
        GLuint attributeIndex = gInstanceAttributeIndex + attributeOffset;

        glEnableVertexAttribArray(attributeIndex);
        // The vertex attributes in the shader are float, so use glVertexAttribPointer.
        glVertexAttribPointer(attributeIndex,
                              4,
                              GL_FLOAT,
                              GL_FALSE, //normalized
                              sizeof(decltype(InstanceList::Instance::aModelTransform)),
                              reinterpret_cast<void *>(sizeof(GLfloat) * 4 * attributeOffset));
        glVertexAttribDivisor(attributeIndex, 1);
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
        mesh.primitives.emplace_back(primitive, mesh.instances);
    }
    return mesh;
}


math::AffineMatrix<4, float> getLocalTransform(const arte::gltf::Node::TRS & aTRS)
{
    return 
        math::trans3d::scale(aTRS.scale.as<math::Size>())
        * aTRS.rotation.toRotationMatrix()
        * math::trans3d::translate(aTRS.translation)
        ;
}


math::AffineMatrix<4, float> getLocalTransform(const arte::gltf::Node & aNode)
{
    if(auto matrix = std::get_if<math::AffineMatrix<4, float>>(&aNode.transformation))
    {
        return *matrix;
    }
    else
    {
        return getLocalTransform(std::get<gltf::Node::TRS>(aNode.transformation));
    }
}


/// \brief A single (non-instanted) draw of the mesh primitive.
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


/// \brief Instantiated rendering of the mesh primitive.
void render(const MeshPrimitive & aMeshPrimitive, GLsizei aInstanceCount)
{
    graphics::bind_guard boundVao{aMeshPrimitive.vao};
    if (aMeshPrimitive.indices)
    {
        ADLOG(gDrawLogger, trace)
             ("Indexed rendering of {} instance(s) of {} vertices with mode {}.",
              aMeshPrimitive.count, aInstanceCount, aMeshPrimitive.drawMode);

        const Indices & indices = *aMeshPrimitive.indices;
        graphics::bind_guard boundIndexBuffer{indices.ibo};
        glDrawElementsInstanced(
            aMeshPrimitive.drawMode, 
            aMeshPrimitive.count,
            indices.componentType,
            reinterpret_cast<void *>(indices.byteOffset),
            aInstanceCount);
    }
    else
    {
        ADLOG(gDrawLogger, trace)
             ("Instanced array rendering of {} instance(s) of {} vertices with mode {}.",
              aMeshPrimitive.count, aInstanceCount, aMeshPrimitive.drawMode);

        glDrawArraysInstanced(
            aMeshPrimitive.drawMode,  
            0, // Start at the beginning of enable arrays, all byte offsets are aleady applied.
            aMeshPrimitive.count,
            aInstanceCount);
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
        gltfviewer::render(primitive, aMesh.instances.size());
    }
}


void Renderer::setCameraTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation)
{
    setUniform(mProgram, "u_camera", aTransformation); 
}


void Renderer::setProjectionTransformation(const math::Matrix<4, 4, GLfloat> & aTransformation)
{
    setUniform(mProgram, "u_projection", aTransformation); 
}


} // namespace gltfviewer
} // namespace ad 