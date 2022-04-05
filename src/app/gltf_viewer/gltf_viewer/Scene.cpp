#include "Scene.h"

#include "Shaders.h"


namespace ad {
namespace gltfviewer {


//
// Joint drawer
//
void JointDrawer::pushJoint(const math::AffineMatrix<4, float> & aTransform) 
{
    math::Position<4, GLfloat> origin{0.f, 0.f, 0.f, 1.f};
    auto jointPosition = (origin * aTransform).xyz();

    if(!jointStack.empty())
    {
        if(options.showSkeletons)
        {
            debugDrawer.addLine({jointStack.back().first, jointPosition, 6, {255, 127, 0, 127}});
        }
        jointStack.back().second = true;
    }

    jointStack.emplace_back(jointPosition, false);
}


void JointDrawer::popJoint(const math::AffineMatrix<4, float> & aTransform)
{
    if(!jointStack.back().second && options.showSkeletons) // if draw did not occur down the line
    {
        // Arbitrary Y position for the "second point" of leaf joints.
        // Maybe there is something better to do to ensure the second point is placed more naturally in the mesh?
        math::Position<4, GLfloat> end{0.f, 1.f, 0.f, 1.f};
        debugDrawer.addLine({jointStack.back().first, (end * aTransform).xyz(), 6, {128, 128, 0, 127}});
    }
    jointStack.pop_back();
}


//
// Programs
//
void Scene::setView(const math::AffineMatrix<4, float> & aViewTransform)
{
    renderer.setCameraTransformation(aViewTransform);
    debugDrawer.setCameraTransformation(aViewTransform);
}


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
