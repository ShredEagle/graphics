#include "Scene.h"

#include "Shaders.h"


namespace ad {
namespace gltfviewer {


//
// Programs
//
void Scene::initializePrograms()
{
    const math::Box<GLfloat> projectedBox =
        graphics::getViewVolumeRightHanded(appInterface->getWindowSize(), 2.f, gViewedDepth, 2*gViewedDepth);
    math::Matrix<4, 4, float> projectionTransform = 
        math::trans3d::orthographicProjection(projectedBox)
        * math::trans3d::scale(1.f, 1.f, -1.f); // OpenGL clipping space is left handed.
    //projectionTransform = math::trans3d::perspective(2.f, -2.f) * projectionTransform;

    {
        auto naive = std::make_shared<graphics::Program>(
            graphics::makeLinkedProgram({
                {GL_VERTEX_SHADER,   gltfviewer::gNaiveVertexShader},
                {GL_FRAGMENT_SHADER, gltfviewer::gNaiveFragmentShader},
            }));
        setUniform(*naive, "u_projection", projectionTransform); 
        shaderPrograms.push_back(std::move(naive));
    }

    {
        auto phong = std::make_shared<graphics::Program>(
            graphics::makeLinkedProgram({
                {GL_VERTEX_SHADER,   gltfviewer::gPhongVertexShader},
                {GL_FRAGMENT_SHADER, gltfviewer::gPhongFragmentShader},
            }));
        setUniform(*phong, "u_projection", projectionTransform); 
        setUniform(*phong, "u_light.position_world", math::Vec<4, GLfloat>{-2.f, 1.f, 6.f, 1.f}); 
        setUniform(*phong, "u_light.color", math::hdr::gWhite<GLfloat>/2.f);
        setUniformInt(*phong, "u_light.specularExponent", 1000);
        setUniformFloat(*phong, "u_light.ambient", 0.4f);
        shaderPrograms.push_back(std::move(phong));
    }
}


void Scene::setProjectionHeight(GLfloat aHeight)
{
    const math::Box<GLfloat> projectedBox =
        graphics::getViewVolumeRightHanded(appInterface->getWindowSize(), aHeight, gViewedDepth, 2*gViewedDepth);
    math::Matrix<4, 4, float> projectionTransform = 
        math::trans3d::orthographicProjection(projectedBox)
        * math::trans3d::scale(1.f, 1.f, -1.f); // OpenGL clipping space is left handed.

    for (auto & program : shaderPrograms)
    {
        setUniform(*program, "u_projection", projectionTransform); 
    }

    currentProjectionHeight = aHeight;
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
        result = indexToMeshes.at(*aNode->mesh).mesh.boundingBox * modelTransform;
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
