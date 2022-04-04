#include "Scene.h"

#include "Shaders.h"


namespace ad {
namespace gltfviewer {


//
// Programs
//
void Scene::setProjection(const math::Matrix<4, 4, float> & aProjectionTransform)
{
    renderer.setProjectionTransformation(aProjectionTransform);
    debugDrawer.setProjectionTransformation(aProjectionTransform);
}


//
// Bounding Box
//
std::optional<math::Box<GLfloat>> & uniteOptionalBoxes(
    std::optional<math::Box<GLfloat>> & aDestination,
    std::optional<math::Box<GLfloat>> aUnited)
{
    if (!aDestination)
    {
        aDestination = aUnited;
    }
    else if(aUnited)
    {
        aDestination->uniteAssign(*aUnited);
    }
    return aDestination;
}


std::optional<math::Box<GLfloat>>
Scene::getBoundingBox(arte::Const_Owned<arte::gltf::Node> aNode,
                      math::AffineMatrix<4, float> aParentTransform) const
{
    math::AffineMatrix<4, float> modelTransform = getLocalTransform(aNode) * aParentTransform;

    std::optional<math::Box<GLfloat>> result;
    if(aNode->mesh)
    {
        result = indexToMesh.at(*aNode->mesh).mesh.boundingBox * modelTransform;
    }
    
    for (auto node : aNode.iterate(&arte::gltf::Node::children))
    {
        uniteOptionalBoxes(result, getBoundingBox(node, modelTransform));
    }
    return result;
}


std::optional<math::Box<GLfloat>>
Scene::getBoundingBox(arte::Const_Owned<arte::gltf::Scene> aScene) const
{
    std::optional<math::Box<GLfloat>> result;
    for (auto node : aScene.iterate(&arte::gltf::Scene::nodes))
    {
        uniteOptionalBoxes(result, getBoundingBox(node));
    }
    return result;
}


} // namespace gltfviewer
} // namespace ad
