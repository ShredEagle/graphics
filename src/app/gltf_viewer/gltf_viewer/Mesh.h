#pragma once


#include <arte/gltf/Gltf.h>

#include <renderer/VertexSpecification.h>

#include <math/Box.h>
#include <math/Homogeneous.h>

#include <renderer/Texture.h>

#include <set>
#include <span>


namespace ad {
namespace gltfviewer {

// TODO Ad 2022/03/29: Implement a better control on buffer dumping, accessible via UI.
constexpr bool gDumpBuffersContent = false;


struct Indices
{
    Indices(arte::Const_Owned<arte::gltf::Accessor> aAccessor);

    graphics::IndexBufferObject ibo;
    GLenum componentType;
    std::size_t byteOffset;
};


struct ViewerVertexBuffer
{
    graphics::VertexBufferObject vbo;
    GLsizei stride;
};


class InstanceList
{
    friend class MeshPrimitive;

public:
    struct Instance
    {
         math::AffineMatrix<4, GLfloat> aModelTransform;
    };

    InstanceList()
    {
        // Ensures there is once instance with indentity model transform until client codes
        // specifies explicit instances.
        std::vector<Instance> defaultInstance{ {math::AffineMatrix<4, GLfloat>::Identity()} };
        update(defaultInstance);
    }

    void update(std::span<Instance> aInstances);

    GLsizei size() const
    { return mInstanceCount; }

private:
    graphics::VertexBufferObject mVbo;
    GLsizei mInstanceCount{0};
};


struct Material
{
    Material(arte::Const_Owned<arte::gltf::Material> aMaterial);

    static arte::gltf::material::PbrMetallicRoughness 
    GetPbr(arte::Const_Owned<arte::gltf::Material> aMaterial);

    math::hdr::Rgba<GLfloat> baseColorFactor;
    std::shared_ptr<graphics::Texture> baseColorTexture;
    arte::gltf::Material::AlphaMode alphaMode;
    bool doubleSided;


    static std::shared_ptr<graphics::Texture> DefaultTexture();
};


struct MeshPrimitive
{
    // Note: Handles caching of sparse accessors.
    // If an accessor is sparse, we cannot reuse any existing buffer for the underlying buffer view.
    // We treat a each sparse accessor as unique from the perspective of the buffer cache.
    // Different non-sparse accessors pointing to the same BufferView will still share the same cached buffer.
    class BufferId
    {
    public:
        BufferId(arte::Const_Owned<arte::gltf::BufferView> aBufferView,
                 arte::Const_Owned<arte::gltf::Accessor> aAccessor);

        bool operator<(const BufferId & aRhs) const;

    private:
        arte::gltf::Index<arte::gltf::BufferView> mBufferView;
        arte::gltf::Index<arte::gltf::Accessor> mAccessor{
            std::numeric_limits<arte::gltf::Index<arte::gltf::Accessor>::Value_t>::max()};
    };

    MeshPrimitive(arte::Const_Owned<arte::gltf::Primitive> aPrimitive);
    MeshPrimitive(arte::Const_Owned<arte::gltf::Primitive> aPrimitive, const InstanceList & aInstances);

    const ViewerVertexBuffer & prepareVertexBuffer(arte::Const_Owned<arte::gltf::Accessor> aAccessor);

    void associateInstanceBuffer(const InstanceList & aInstances);

    /// \brief Returns true if this mesh primitive has vertex color provided.
    bool providesColor() const;

    // NOTE Ad 2022/03/04: I wanted to use this occasion to brush-up knowledge of OpenGL functions.
    // So instead of using the abstractions in render libraries, do most of the calls directly.
    //VertexSpecification vertexSpecification;

    GLenum drawMode;
    GLsizei count{0};
    graphics::VertexArrayObject vao;
    // Note: Decision that OpenGL buffers would map to glTF buffer views:
    // * This should allow to respect the interleaving, as it usually seems achieved
    //   via several accessors (with different offsets) on the same 
    //   buffer view (with a stride), at least in the examples I've seen.
    // * This should result in distinct GL buffers when the same buffer
    //   contains vertex attributes and indices, accessed via different buffer views.
    // This will not be foolproof, as I suspect interleaving can be achieved via
    // several buffer views, for example.
    std::map<BufferId, ViewerVertexBuffer> vbos;
    std::optional<Indices> indices;
    std::set<GLuint> providedAttributes;

    Material material;
    math::Box<GLfloat> boundingBox{{0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}};
};


struct Mesh
{
    std::vector<MeshPrimitive> primitives;
    math::Box<GLfloat> boundingBox{{0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}};
    InstanceList gpuInstances;
};


Mesh prepare(arte::Const_Owned<arte::gltf::Mesh> aMesh);

std::shared_ptr<graphics::Texture> prepare(arte::Const_Owned<arte::gltf::Texture> aTexture);


std::ostream & operator<<(std::ostream & aOut, const MeshPrimitive &);
std::ostream & operator<<(std::ostream & aOut, const Mesh &);


} // namespace gltfviewer
} // namespace ad
