#include "GltfRendering.h"

#include "Logging.h"

#include <renderer/GL_Loader.h>
#include <renderer/VertexSpecification.h>


namespace ad {

using namespace arte;

namespace gltfviewer {


struct MeshPrimitive
{
    MeshPrimitive(Const_Owned<gltf::Primitive> aPrimitive);

    GLenum drawMode;
    bool indexed;
    //VertexSpecification vertexSpecification;
    graphics::VertexArrayObject vao;
    std::vector<graphics::VertexBufferObject> vbos;
};


MeshPrimitive::MeshPrimitive(Const_Owned<gltf::Primitive> aPrimitive)
{
    glBindVertexArray(vao);
    for (const auto & [semantic, accessorIndex] : aPrimitive.elem().attributes)
    {
        const gltf::Accessor & accessor = aPrimitive.get(accessorIndex);
        ADLOG(gMainLogger, debug)("Accessor {}: {}", semantic, accessorIndex);
    }
}


void render(arte::Const_Owned<arte::gltf::Mesh> aMesh)
{
    for (auto & primitive : aMesh.iterate(&arte::gltf::Mesh::primitives))     
    {
        MeshPrimitive meshPrimitive{primitive};
    }
}


} // namespace gltfviewer
} // namespace ad
