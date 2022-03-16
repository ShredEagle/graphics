#pragma once


#include <arte/Gltf.h>

#include <math/Homogeneous.h>

#include <renderer/VertexSpecification.h>

#include <renderer/Shading.h>


namespace ad {
namespace gltfviewer {


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


struct MeshPrimitive
{
    MeshPrimitive(arte::Const_Owned<arte::gltf::Primitive> aPrimitive);

    const ViewerVertexBuffer & prepareVertexBuffer(arte::Const_Owned<arte::gltf::BufferView> aBufferView);

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
    std::map<arte::gltf::Index<arte::gltf::BufferView>, ViewerVertexBuffer> vbos;
    std::optional<Indices> indices;
};


struct Mesh
{
    std::vector<MeshPrimitive> primitives;
};


std::ostream & operator<<(std::ostream & aOut, const MeshPrimitive &);
std::ostream & operator<<(std::ostream & aOut, const Mesh &);


Mesh prepare(arte::Const_Owned<arte::gltf::Mesh> aMesh);


math::AffineMatrix<4, float> getLocalTransform(arte::gltf::Node aNode);


class Renderer
{
    // TODO remove this friendship once the api is complete
    friend class Scene;

public:
    Renderer();

    void setCameraTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation);
    void setProjectionTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation);

    void render(const Mesh & aMesh) const;

private:
    graphics::Program mProgram;
};

} // namespace gltfviewer
} // namespace ad
