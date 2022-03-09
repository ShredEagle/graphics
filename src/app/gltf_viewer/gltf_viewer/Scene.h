#pragma once


#include "GltfRendering.h"
#include "Polar.h"

#include <graphics/AppInterface.h>
#include <graphics/CameraUtilities.h>

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
    Scene(arte::Const_Owned<arte::gltf::Scene> aScene,
          std::shared_ptr<graphics::AppInterface> aAppInterface) :
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
        ADLOG(gDrawLogger, debug)("Camera position {}.", cameraCartesian);

        math::Vec<3, GLfloat> gazeDirection = gGazePoint - cameraCartesian;
        renderer.setCameraTransformation(
            graphics::getCameraTransform(cameraCartesian,
                                         gazeDirection,
                                         cameraPosition.getUpTangent()));
    }

    void render() const
    {
        for (const auto & [id, mesh] : indexToMeshes)
        {
            renderer.render(mesh);
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
