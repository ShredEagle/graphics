#pragma once


#include "Camera.h"
#include "DebugDrawer.h"
#include "GltfAnimation.h"
#include "GltfRendering.h"
#include "Logging.h"
#include "Mesh.h"
#include "Polar.h"
#include "SkeletalAnimation.h"

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
    std::vector<arte::gltf::Index<arte::gltf::Skin>> skinInstances;
};

// Associates a mesh index to a mesh loaded on the Gpu
using MeshRepository = std::map<arte::gltf::Index<arte::gltf::Mesh>, MeshInstances>;
// Associates a skin index to a Skeleton with its uniform buffer on the Gpu
using SkeletonRepository = std::map<arte::gltf::Index<arte::gltf::Skin>, Skeleton>;

using AnimationRepository = std::vector<Animation>;


inline void clearInstances(MeshRepository & aRepository)
{
    for(auto & [index, mesh] : aRepository)
    {
        mesh.instances.clear();
        mesh.skinInstances.clear();
    }
}


/// \brief Associate a gltf::mesh index to a viewer's Mesh instance.
template <class T_nodeRange>
void populateMeshRepository(MeshRepository & aRepository,
                            SkeletonRepository & aSkeletonRepo,
                            const T_nodeRange & aNodes)
{
    for (arte::Owned<arte::gltf::Node> node : aNodes)
    {
        if(node->mesh)
        {
            if(!aRepository.contains(*node->mesh))
            {
                auto [it, didInsert] = aRepository.emplace(
                    *node->mesh,
                    prepare(node.get(&arte::gltf::Node::mesh)));
                ADLOG(gPrepareLogger, info)("Completed GPU loading for mesh '{}'.", it->second.mesh);
            }
            // Only populates skins that are actually present in this scene.
            if(node->skin && !aSkeletonRepo.contains(*node->skin))
            {
                aSkeletonRepo.emplace(*node->skin, Skeleton{node.get(&arte::gltf::Node::skin)});
                ADLOG(gPrepareLogger, debug)("Loaded skeleton for skin #{}.", *node->skin);
            }
        }
        populateMeshRepository(aRepository, aSkeletonRepo, node.iterate(&arte::gltf::Node::children));
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


struct UserOptions
{
    bool showSkeletons{true};
};


struct JointDrawer
{
    void pushJoint(const math::AffineMatrix<4, float> & aTransform);
    void popJoint(const math::AffineMatrix<4, float> & aTransform);

    DebugDrawer & debugDrawer;
    UserOptions & options;
    std::vector<std::pair<math::Position<3, GLfloat>, bool/*draw occurred*/>> jointStack;
};



struct Scene
{
    Scene(arte::Gltf aGltf,
          arte::gltf::Index<arte::gltf::Scene> aSceneIndex,
          std::shared_ptr<graphics::AppInterface> aAppInterface) :
        gltf{std::move(aGltf)},
        scene{gltf.get(aSceneIndex)},
        appInterface{std::move(aAppInterface)},
        camera{appInterface},
        debugDrawer{appInterface}
    {
        populateMeshRepository(indexToMesh, 
                               indexToSkeleton,
                               scene.iterate(&arte::gltf::Scene::nodes));
        populateAnimationRepository(animations, gltf.getAnimations());
        if (!animations.empty())
        {
            activeAnimation = &animations.front();
        }
        
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

    void setView(const math::AffineMatrix<4, float> & aViewTransform);
    void setProjection(const math::Matrix<4, 4, float> & aProjectionTransform);

    void update(const graphics::Timer & aTimer)
    {
        setView(camera.update());

        updateAnimation(aTimer);
        updatesInstances();
    }


    /// \brief Modify gltf Nodes with the updated local transforms.
    void updateAnimation(const graphics::Timer & aTimer)
    {
        if(activeAnimation != nullptr)
        {
            activeAnimation->updateScene(aTimer.time(), scene);
        }
    }


    void updatesInstances()
    {
        clearInstances(indexToMesh);
        JointDrawer jointDrawer{.debugDrawer = debugDrawer, .options = options};
        for (auto node : scene.iterate(&arte::gltf::Scene::nodes))
        {
            updatesInstances(node, jointDrawer);
        }

        for(auto & [_index, mesh] : indexToMesh)
        {
            // Update the VBO containing instance data with the client vector of instance data
            mesh.mesh.gpuInstances.update(mesh.instances);
        }

        for (auto & [_index, skeleton] : indexToSkeleton)
        {
            skeleton.updatePalette(nodeToJoint);
        }
    }


    // Recursive function to:
    // * queue the mesh instance
    // * traverse the node children
    void updatesInstances(arte::Owned<arte::gltf::Node> aNode,
                          JointDrawer & aJointDrawer,
                           math::AffineMatrix<4, float> aParentTransform = 
                               math::AffineMatrix<4, float>::Identity())
    {
        math::AffineMatrix<4, float> modelTransform = getLocalTransform(aNode) * aParentTransform;

        if(aNode->mesh)
        {
            if(aNode->skin)
            {
                indexToMesh.at(*aNode->mesh).skinInstances.push_back(*aNode->skin);
            }
            else
            {
                indexToMesh.at(*aNode->mesh).instances.push_back({modelTransform});
            }
        }

        if(aNode->usedAsJoint)
        {
            nodeToJoint.insert_or_assign(aNode.id(), Joint{modelTransform});
            aJointDrawer.pushJoint(modelTransform);
        }

        for (auto node : aNode.iterate(&arte::gltf::Node::children))
        {
            updatesInstances(node, aJointDrawer, modelTransform);
        }

        if(aNode->usedAsJoint)
        {
            aJointDrawer.popJoint(modelTransform);
        }
    }


    void render() const
    {
        for(const auto & [index, mesh] : indexToMesh)
        {
            // Render "static" instances
            if(mesh.instances.size() != 0)
            {
                renderer.render(mesh.mesh);
            }

            // Render skinned instances
            for (auto skinId : mesh.skinInstances)
            {
                renderer.render(mesh.mesh, indexToSkeleton.at(skinId));
            }
        }

        debugDrawer.render();
    }


    void callbackKeyboard(int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            appInterface->requestCloseApplication();
        }

        else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        {
            renderer.togglePolygonMode();
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

    arte::Gltf gltf;
    arte::Owned<arte::gltf::Scene> scene;
    MeshRepository indexToMesh;
    SkeletonRepository indexToSkeleton;
    AnimationRepository animations;
    Animation * activeAnimation{nullptr};
    JointRepository nodeToJoint;
    Renderer renderer;
    std::shared_ptr<graphics::AppInterface> appInterface;
    UserCamera camera;
    UserOptions options;
    DebugDrawer debugDrawer;

    static constexpr GLfloat gViewportHeightFactor = 1.6f;
    static constexpr GLfloat gScrollFactor = 0.05;
};

} // namespace gltfviewer
} // namespace ad
