#pragma once


#include "GltfRendering.h"
#include "Polar.h"

#include <graphics/AppInterface.h>
#include <graphics/CameraUtilities.h>

#include <renderer/Uniforms.h>

#include <math/Box.h>

// Dirty include, to get the GLFW input definitions
#include <GLFW/glfw3.h>


namespace ad {
namespace gltfviewer {


template <class T_nodeRange>
void populateMeshRepository(MeshRepository & aRepository, const T_nodeRange & aNodes)
{
    for (arte::Const_Owned<arte::gltf::Node> node : aNodes)
    {
        if(node->mesh && !aRepository.contains(*node->mesh))
        {
            auto [it, didInsert] = 
                aRepository.emplace(*node->mesh, prepare(node.get(&arte::gltf::Node::mesh)));
            ADLOG(gPrepareLogger, info)("Completed GPU loading for mesh '{}'.", it->second);
        }
        populateMeshRepository(aRepository, node.iterate(&arte::gltf::Node::children));
    }
}


struct Scene
{
    Scene(std::shared_ptr<arte::Gltf> aGltf,
          arte::Const_Owned<arte::gltf::Scene> aScene,
          std::shared_ptr<graphics::AppInterface> aAppInterface) :
        gltf{std::move(aGltf)},
        scene{aScene},
        appInterface{std::move(aAppInterface)}
    {
        populateMeshRepository(indexToMeshes, aScene.iterate(&arte::gltf::Scene::nodes));

        const math::Box<GLfloat> aProjectedBox =
            graphics::getViewVolume(appInterface->getWindowSize(), 2.f, 0.f, gViewedDepth);
        renderer.setProjectionTransformation(math::trans3d::orthographicProjection(aProjectedBox));

        using namespace std::placeholders;
        appInterface->registerKeyCallback(
            std::bind(&Scene::callbackKeyboard, this, _1, _2, _3, _4));
        appInterface->registerMouseButtonCallback(
            std::bind(&Scene::callbackMouseButton, this, _1, _2, _3, _4, _5));
        appInterface->registerCursorPositionCallback(
            std::bind(&Scene::callbackCursorPosition, this, _1, _2));
    }

    void update()
    {
        const math::Position<3, GLfloat> cameraCartesian = cameraPosition.toCartesian();
        ADLOG(gDrawLogger, trace)("Camera position {}.", cameraCartesian);

        math::Vec<3, GLfloat> gazeDirection = gGazePoint - cameraCartesian;
        renderer.setCameraTransformation(
            graphics::getCameraTransform(cameraCartesian,
                                         gazeDirection,
                                         cameraPosition.getUpTangent()));
    }

    // TODO make const when update populates instances
    void render() //const
    {
        for (auto node : scene.iterate(&arte::gltf::Scene::nodes))
        {
            render(node);
        }
    }

    // Recursive function, rendering the node mesh then traversing the node children.
    void render(arte::Const_Owned<arte::gltf::Node> aNode,
                math::AffineMatrix<4, float> aParentTransform = math::AffineMatrix<4, float>::Identity()) //const
    {
        math::AffineMatrix<4, float> modelTransform = getLocalTransform(aNode) * aParentTransform;
        if(aNode->mesh)
        {
            graphics::setUniform(renderer.mProgram, "u_model", modelTransform); 
            renderer.render(indexToMeshes.at(*aNode->mesh));
        }
        for (auto node : aNode.iterate(&arte::gltf::Node::children))
        {
            render(node, modelTransform);
        }
    }

    void callbackKeyboard(int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            appInterface->requestCloseApplication();
        }

        else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        {
            switch(polygonMode)
            {
            case PolygonMode::Fill:
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                polygonMode = PolygonMode::Line;
                break;
            case PolygonMode::Line:
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                polygonMode = PolygonMode::Fill;
                break;
            }
        }
    }

    void callbackMouseButton(int button, int action, int mods, double xpos, double ypos)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            previousDragPosition = math::Position<2, GLfloat>{(GLfloat)xpos, (GLfloat)ypos};
        }
        else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        {
            previousDragPosition = std::nullopt;
        }
    }

    void callbackCursorPosition(double xpos, double ypos)
    {
        using Radian = math::Radian<GLfloat>;
        // top-left corner origin
        if (previousDragPosition)
        {
            math::Position<2, GLfloat> cursorPosition{(GLfloat)xpos, (GLfloat)ypos};
            auto angularIncrements = (cursorPosition - *previousDragPosition).cwMul(gMouseControlFactor);

            cameraPosition.azimuthal += Radian{angularIncrements.x()};
            cameraPosition.polar += Radian{angularIncrements.y()};
            cameraPosition.polar = std::max(Radian{0}, std::min(Radian{math::pi<GLfloat>}, cameraPosition.polar));

            previousDragPosition = cursorPosition;
        }
    }

    enum class PolygonMode
    {
        Fill,
        Line,
    };

    std::shared_ptr<arte::Gltf> gltf;
    const arte::Const_Owned<arte::gltf::Scene> & scene;
    Polar cameraPosition{2.f};
    MeshRepository indexToMeshes;
    Renderer renderer;
    PolygonMode polygonMode{PolygonMode::Fill};
    std::shared_ptr<graphics::AppInterface> appInterface;
    std::optional<math::Position<2, GLfloat>> previousDragPosition;

    static constexpr math::Position<3, GLfloat> gGazePoint{0.f, 0.f, 0.f};
    static constexpr math::Vec<2, GLfloat> gMouseControlFactor{1/700.f, 1/700.f};
    static constexpr GLfloat gViewedDepth = 10000;
};

} // namespace gltfviewer
} // namespace ad
