#include "GltfRendering.h"

#include "Logging.h"

#include <renderer/GL_Loader.h>
#include <renderer/VertexSpecification.h>


namespace ad {

using namespace arte;

namespace gltfviewer {


struct MeshPrimitive
{
    MeshPrimitive(const gltf::Primitive & aPrimitive);

    GLenum drawMode;
    bool indexed;
    //VertexSpecification vertexSpecification;
    graphics::VertexArrayObject vao;
    std::vector<graphics::VertexBufferObject> vbos;
};


MeshPrimitive::MeshPrimitive(const gltf::Primitive & aPrimitive)
{
    glBindVertexArray(vao);
    for (const auto & [semantic, accessorIndex] : aPrimitive.attributes)
    {
        ADLOG(gMainLogger, debug)("Accessor index: {}", accessorIndex);
    }
}


void render(const gltf::Mesh & aMesh)
{
    for (const auto & primitive : aMesh.primitives)     
    {
        MeshPrimitive meshPrimitive{primitive};
    }
}


} // namespace gltfviewer
} // namespace ad
