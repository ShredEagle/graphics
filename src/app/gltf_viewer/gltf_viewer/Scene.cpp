#include "Scene.h"

#include "Shaders.h"


namespace ad {
namespace gltfviewer {


void Scene::initializePrograms()
{
    const math::Box<GLfloat> projectedBox =
        //graphics::getViewVolume(appInterface->getWindowSize(), 2.f, 0, gViewedDepth);
        graphics::getViewVolume(appInterface->getWindowSize(), 2.f, -gViewedDepth, -gViewedDepth);
    math::Matrix<4, 4, float> projectionTransform = math::trans3d::orthographicProjection(projectedBox);
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
        setUniform(*phong, "u_light.position_world", math::Vec<4, GLfloat>{-2.f, 3.f, 8.f, 1.f}); 
        setUniform(*phong, "u_light.color", math::hdr::gWhite<GLfloat>/2.5f);
        setUniformInt(*phong, "u_light.specularExponent", 1000);
        setUniformFloat(*phong, "u_light.ambient", 0.3f);
        shaderPrograms.push_back(std::move(phong));
    }
}


} // namespace gltfviewer
} // namespace ad
