#include "Scene.h"

#include "Shaders.h"


namespace ad {
namespace gltfviewer {


//
// Programs
//
void Scene::initializePrograms()
{
    {
        auto naive = std::make_shared<graphics::Program>(
            graphics::makeLinkedProgram({
                {GL_VERTEX_SHADER,   gltfviewer::gNaiveVertexShader},
                {GL_FRAGMENT_SHADER, gltfviewer::gNaiveFragmentShader},
            }));
        shaderPrograms.push_back(std::move(naive));
    }

    {
        auto phong = std::make_shared<graphics::Program>(
            graphics::makeLinkedProgram({
                {GL_VERTEX_SHADER,   gltfviewer::gPhongVertexShader},
                {GL_FRAGMENT_SHADER, gltfviewer::gPhongFragmentShader},
            }));
        setUniform(*phong, "u_light.position_world", math::Vec<4, GLfloat>{-100.f, 100.f, 1000.f, 1.f}); 
        setUniform(*phong, "u_light.color", math::hdr::gWhite<GLfloat>);
        setUniformInt(*phong, "u_light.specularExponent", 100);
        // Ideally, I suspect the sum should be 1
        setUniformFloat(*phong, "u_light.diffuse", 0.3f);
        setUniformFloat(*phong, "u_light.specular", 0.35f);
        setUniformFloat(*phong, "u_light.ambient", 0.45f);
        shaderPrograms.push_back(std::move(phong));
    }
}


void Scene::setProjection(const math::Matrix<4, 4, float> & aProjectionTransform)
{
    for (auto & program : shaderPrograms)
    {
        setUniform(*program, "u_projection", aProjectionTransform); 
    }
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
