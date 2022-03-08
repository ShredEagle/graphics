#pragma once


#include <arte/Gltf.h>

#include <renderer/VertexSpecification.h>


namespace ad {
namespace gltfviewer {


struct Indices
{
    Indices(arte::Const_Owned<arte::gltf::Accessor> aAccessor);

    graphics::IndexBufferObject ibo;
    GLenum componentType;
    std::size_t byteOffset;
};


struct MeshPrimitive
{
    MeshPrimitive(arte::Const_Owned<arte::gltf::Primitive> aPrimitive);

    // NOTE Ad 2022/03/04: I wanted to use this occasion to brush-up knowledge of OpenGL functions.
    // So instead of using the abstractions in render libraries, do most of the calls directly.
    //VertexSpecification vertexSpecification;

    GLenum drawMode;
    GLsizei count{0};
    graphics::VertexArrayObject vao;
    std::vector<graphics::VertexBufferObject> vbos;
    std::optional<Indices> indices;
};


struct Mesh
{
    std::vector<MeshPrimitive> primitives;
};


std::ostream & operator<<(std::ostream & aOut, const MeshPrimitive &);
std::ostream & operator<<(std::ostream & aOut, const Mesh &);



using MeshRepository = std::map<arte::gltf::Index<arte::gltf::Mesh>, Mesh>;

Mesh prepare(arte::Const_Owned<arte::gltf::Mesh> aMesh);

void render(const Mesh & aMesh);


} // namespace gltfviewer
} // namespace ad
