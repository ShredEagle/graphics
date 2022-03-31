#pragma once


#include "GltfAnimation.h"
#include "GltfRendering.h"
#include "Logging.h"
#include "Polar.h"
#include "Camera.h"

#include <graphics/AppInterface.h>
#include <graphics/Timer.h>

#include <renderer/Uniforms.h>

#include <math/Box.h>


namespace ad {
namespace gltfviewer {


struct MeshInstances
{
    Mesh mesh;
    std::vector<InstanceList::Instance> instances; 
};


using MeshRepository = std::map<arte::gltf::Index<arte::gltf::Mesh>,
                                MeshInstances>;
using AnimationRepository = std::vector<Animation>;


inline void clearInstances(MeshRepository & aRepository)
{
    for(auto & [index, mesh] : aRepository)
    {
        mesh.instances.clear();
    }
}


/// \brief Associate a gltf::mesh index to a viewer's Mesh instance.
template <class T_nodeRange>
void populateMeshRepository(MeshRepository & aRepository, const T_nodeRange & aNodes)
{
    for (arte::Owned<arte::gltf::Node> node : aNodes)
    {
        if(node->mesh && !aRepository.contains(*node->mesh))
        {
            auto [it, didInsert] = 
                aRepository.emplace(*node->mesh,
                                    MeshInstances{.mesh =prepare(node.get(&arte::gltf::Node::mesh))});
            ADLOG(gPrepareLogger, info)("Completed GPU loading for mesh '{}'.", it->second.mesh);
        }
        populateMeshRepository(aRepository, node.iterate(&arte::gltf::Node::children));
    }
}


/// \brief Create viewer's Animation instances for each animation in the provided range.
template <class T_animationRange>
void populateAnimationRepository(AnimationRepository & aRepository, const T_animationRange & aAnimations)
{
    for (arte::Owned<arte::gltf::Animation> animation : aAnimations)
    {
        aRepository.push_back(prepare(animation));
    }
}


struct Scene
{
    Scene(arte::Gltf aGltf,
          arte::gltf::Index<arte::gltf::Scene> aSceneIndex,
          std::shared_ptr<graphics::AppInterface> aAppInterface) :
        gltf{std::move(aGltf)},
        scene{gltf.get(aSceneIndex)},
        appInterface{std::move(aAppInterface)},
        camera{appInterface}
    {
        populateMeshRepository(indexToMeshes, scene.iterate(&arte::gltf::Scene::nodes));
        populateAnimationRepository(animations, gltf.getAnimations());
        if (!animations.empty())
        {
            activeAnimation = &animations.front();
        }

        initializePrograms();
        renderer.changeProgram(shaderPrograms.at(currentProgram));
        
        // Not enabled by default OpenGL context.
        glEnable(GL_DEPTH_TEST);

        auto sceneBounds = getBoundingBox(scene);
        if (!sceneBounds)
        {
            throw std::logic_error{"Scene does not contain bounded geometry to render."};
        }
        ADLOG(gPrepareLogger, info)
             ("Centering camera on {}, scene bounding box is {}.",
              sceneBounds->center(), *sceneBounds);
        camera.setOrigin(sceneBounds->center());
        setProjection(camera.setViewedHeight(sceneBounds->height() * gViewportHeightFactor));


        using namespace std::placeholders;
        appInterface->registerKeyCallback(
            std::bind(&Scene::callbackKeyboard, this, _1, _2, _3, _4));
        appInterface->registerMouseButtonCallback(
            std::bind(&Scene::callbackMouseButton, this, _1, _2, _3, _4, _5));
        appInterface->registerCursorPositionCallback(
            std::bind(&Scene::callbackCursorPosition, this, _1, _2));
        appInterface->registerScrollCallback(
            std::bind(&Scene::callbackScroll, this, _1, _2));
    }

    std::optional<math::Box<GLfloat>>
    getBoundingBox(arte::Const_Owned<arte::gltf::Scene> aScene) const;
    std::optional<math::Box<GLfloat>>
    getBoundingBox(arte::Const_Owned<arte::gltf::Node> aNode,
                   math::AffineMatrix<4, float> aParentTransform 
                    = math::AffineMatrix<4, float>::Identity()) const;

    void initializePrograms();

    void setProjection(const math::Matrix<4, 4, float> & aProjectionTransform);

    void update(const graphics::Timer & aTimer)
    {
        renderer.setCameraTransformation(camera.update());
        updateAnimation(aTimer);
        updatesInstances();
    }


    void updateAnimation(const graphics::Timer & aTimer)
    {
        if(activeAnimation != nullptr)
        {
            activeAnimation->updateScene(aTimer.time(), scene);
        }
    }

    void updatesInstances()
    {
        clearInstances(indexToMeshes);
        for (auto node : scene.iterate(&arte::gltf::Scene::nodes))
        {
            updatesInstances(node);
        }

        for(auto & [index, pair] : indexToMeshes)
        {
            // Update the VBO containing instance data with the client vector of instance data
            pair.mesh.gpuInstances.update(pair.instances);
        }
    }

    // Recursive function to:
    // * queue the mesh instance
    // * traverse the node children
    void updatesInstances(arte::Owned<arte::gltf::Node> aNode,
                           math::AffineMatrix<4, float> aParentTransform = 
                               math::AffineMatrix<4, float>::Identity())
    {
        math::AffineMatrix<4, float> modelTransform = getLocalTransform(aNode) * aParentTransform;

        if(aNode->mesh)
        {
            indexToMeshes.at(*aNode->mesh).instances.push_back({modelTransform});
        }

        for (auto node : aNode.iterate(&arte::gltf::Node::children))
        {
            updatesInstances(node, modelTransform);
        }
    }


    void render() const
    {
        // TODO should it be optimized to only call when there is at least once instance?
        for(const auto & [index, mesh] : indexToMeshes)
        {
            renderer.render(mesh.mesh);
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

        else if (key == GLFW_KEY_P && action == GLFW_PRESS)
        {
            ++currentProgram;
            currentProgram %= shaderPrograms.size();
            renderer.changeProgram(shaderPrograms.at(currentProgram));
        }
    }

    void callbackCursorPosition(double xpos, double ypos)
    {
        camera.callbackCursorPosition(xpos, ypos);
    }

    void callbackMouseButton(int button, int action, int mods, double xpos, double ypos)
    {
        camera.callbackMouseButton(button, action, mods, xpos, ypos);
    }

    void callbackScroll(double xoffset, double yoffset)
    {
        setProjection(camera.multiplyViewedHeight(1 - yoffset * gScrollFactor));
    }

    enum class PolygonMode
    {
        Fill,
        Line,
    };

    arte::Gltf gltf;
    arte::Owned<arte::gltf::Scene> scene;
    MeshRepository indexToMeshes;
    AnimationRepository animations;
    Animation * activeAnimation{nullptr};
    Renderer renderer;
    PolygonMode polygonMode{PolygonMode::Fill};
    std::vector<std::shared_ptr<graphics::Program>> shaderPrograms;
    std::size_t currentProgram{1};
    std::shared_ptr<graphics::AppInterface> appInterface;
    UserCamera camera;

    static constexpr GLfloat gViewportHeightFactor = 1.6f;
    static constexpr GLfloat gScrollFactor = 0.05;
};

} // namespace gltfviewer
} // namespace ad
