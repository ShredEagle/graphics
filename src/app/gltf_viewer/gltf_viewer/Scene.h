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
};

//struct SkinnedInstance
//{
//    public:
//        SkinnedInstance(arte::gltf::Index<arte::gltf::Mesh> aMeshId,
//                        arte::gltf::Index<arte::gltf::Skin> aSkinId);
//
//        bool operator<(const SkinnedInstance & aRhs) const;
//
//    private:
//        arte::gltf::Index<arte::gltf::Mesh> mMesh;
//        arte::gltf::Index<arte::gltf::Skin> mSkin{
//            std::numeric_limits<arte::gltf::Index<arte::gltf::Skin>::Value_t>::max()};
//};
//using InstanceRepository = std::map<SkinnedInstance, std::vector<InstanceList::Instance>>;

// Associates a mesh index to a mesh loaded on the Gpu
using MeshRepository = std::map<arte::gltf::Index<arte::gltf::Mesh>, MeshInstances>;
using InstanceRepository = std::map<arte::gltf::Index<arte::gltf::Mesh>, std::vector<InstanceList::Instance>>;

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
                                    prepare(node.get(&arte::gltf::Node::mesh), node->skin.has_value()));
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


struct UserOptions
{
    bool showSkeletons{true};
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
        /*TOSK*/skeleton{gltf.get(arte::gltf::Index<arte::gltf::Skin>{0})},
        debugDrawer{appInterface}
    {
        populateMeshRepository(indexToMesh, scene.iterate(&arte::gltf::Scene::nodes));
        populateAnimationRepository(animations, gltf.getAnimations());
        if (!animations.empty())
        {
            activeAnimation = &animations.front();
        }
        
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
        auto view = camera.update();
        renderer.setCameraTransformation(view);
        debugDrawer.setCameraTransformation(view);

        updateAnimation(aTimer);
        updatesInstances();

        /*TOSK*/skeleton.updatePalette(nodeToJoint);

        // TODO remove
        debugDrawer.addLine({{0.f, 0.f, 0.f}, {10.f, 0.f, 0.f}, 4});
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
        clearInstances(indexToMesh);
        for (auto node : scene.iterate(&arte::gltf::Scene::nodes))
        {
            updatesInstances(node);
        }

        for(auto & [index, pair] : indexToMesh)
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
            indexToMesh.at(*aNode->mesh).instances.push_back({modelTransform});
        }

        if(aNode->usedAsJoint)
        {
            nodeToJoint.insert_or_assign(aNode.id(), Joint{modelTransform});

            math::Position<4, GLfloat> origin{0.f, 0.f, 0.f, 1.f};
            math::Position<4, GLfloat> end{0.f, 1.f, 0.f, 1.f};
            debugDrawer.addLine({(origin * modelTransform).xyz(), (end * modelTransform).xyz(), 6, {255, 127, 0, 127}});
        }

        for (auto node : aNode.iterate(&arte::gltf::Node::children))
        {
            updatesInstances(node, modelTransform);
        }
    }


    void render() const
    {
        // TODO should it be optimized to only call when there is at least once instance?
        for(const auto & [index, mesh] : indexToMesh)
        {
            renderer.render(mesh.mesh);
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
    AnimationRepository animations;
    Animation * activeAnimation{nullptr};
    JointRepository nodeToJoint;
    /*TOSK*/Skeleton skeleton;
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
