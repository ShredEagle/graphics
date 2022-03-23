#pragma once


#include "Owned.h"

#include <math/Color.h>
#include <math/Homogeneous.h>
#include <math/Quaternion.h>

#include <platform/Filesystem.h>

#include <functional>
#include <map>
#include <optional>
#include <variant>
#include <vector>


namespace ad {
namespace arte {


namespace gltf {

    using EnumType = unsigned int; // this seems to be GLenum type.

    namespace material
    {
        struct PbrMetallicRoughness
        {
            math::hdr::Rgba<float> baseColorFactor{math::hdr::gWhite<float>};
        };
    } // namespace material

    constexpr material::PbrMetallicRoughness gDefaultMaterial;

    struct Material
    {
        std::string name;
        std::optional<material::PbrMetallicRoughness> pbrMetallicRoughness;
    };

    struct Accessor;

    struct Sampler
    {
        enum class Interpolation
        {
            Linear,
            Step,
            CubicSpline,
        };

        Index<Accessor> input;
        Interpolation interpolation;
        Index<Accessor> output;
    };

    struct Node;

    struct Target
    {
        enum class Path
        {
            Translation,
            Rotation,
            Scale,
            Weights,
        };

        std::optional<Index<Node>> node;
        Path path;
    };

    struct Channel
    {
        Index<Sampler> sampler;
        Target target;
    };

    struct Animation
    {
        std::string name;
        std::vector<Channel> channels;
        std::vector<Sampler> samplers;
    };

    struct Buffer
    {
        std::string name;
        // see: https://github.com/KhronosGroup/glTF/issues/828#issuecomment-277317217
        std::optional<Uri> uri;
        std::size_t byteLength;
    };

    struct BufferView
    {
        std::string name;
        Index<Buffer> buffer;
        std::size_t byteOffset;
        std::size_t byteLength;
        std::optional<std::size_t> byteStride;
        // see: https://github.com/KhronosGroup/glTF/issues/1440
        std::optional<EnumType> target;
    };

    struct Accessor
    {
        enum class ElementType
        {
            Scalar,
            Vec2,
            Vec3,
            Vec4,
            Mat2,
            Mat3,
            Mat4,
        };

        std::string name;
        // When absent, the accessor must be initialized with zeros 
        // (which can be overrided by sparse or extensions)
        std::optional<Index<BufferView>> bufferView;
        std::size_t byteOffset;
        ElementType type;
        EnumType componentType;
        bool normalized;
        std::size_t count;
        // TODO Handle min-max
        // TODO Handle sparse
    };

    struct Primitive
    {
        EnumType mode;
        std::map<std::string, Index<Accessor>> attributes;
        std::optional<Index<Accessor>> indices;
        std::optional<Index<Material>> material;
    };

    struct Mesh
    {
        std::vector<Primitive> primitives;
    };

    struct Node
    {
        using Matrix = math::AffineMatrix<4, float>;
        struct TRS
        {
            math::Vec<3, float> translation;
            math::Quaternion<float> rotation;
            math::Vec<3, float> scale;
        };

        std::string name;
        std::vector<Index<Node>> children;
        std::variant<Matrix, TRS> transformation;
        // TODO Handle non-mesh nodes
        std::optional<Index<Mesh>> mesh;
    };

    struct Scene
    {
        std::string name;
        std::vector<Index<Node>> nodes;
    };

    std::ostream & operator<<(std::ostream & aOut, const Scene & aScene);

} // namespace gltf


std::string to_string(gltf::Accessor::ElementType aElementType);
std::string to_string(gltf::Sampler::Interpolation aInterpolation);


class Gltf
{
public:
    explicit Gltf(const filesystem::path & aGltfJson);

    std::optional<Owned<gltf::Scene>> getDefaultScene();
    std::optional<Const_Owned<gltf::Scene>> getDefaultScene() const;

    std::vector<Owned<gltf::Animation>> getAnimations();
    std::vector<Const_Owned<gltf::Animation>> getAnimations() const;

    Owned<gltf::Accessor> get(gltf::Index<gltf::Accessor> aAccessorIndex);
    Const_Owned<gltf::Accessor> get(gltf::Index<gltf::Accessor> aAccessorIndex) const;

    Owned<gltf::Buffer> get(gltf::Index<gltf::Buffer> aBufferIndex);
    Const_Owned<gltf::Buffer> get(gltf::Index<gltf::Buffer> aBufferIndex) const;

    Owned<gltf::BufferView> get(gltf::Index<gltf::BufferView> aBufferViewIndex);
    Const_Owned<gltf::BufferView> get(gltf::Index<gltf::BufferView> aBufferViewIndex) const;

    Owned<gltf::Mesh> get(gltf::Index<gltf::Mesh> aMeshIndex);
    Const_Owned<gltf::Mesh> get(gltf::Index<gltf::Mesh> aMeshIndex) const;

    Owned<gltf::Node> get(gltf::Index<gltf::Node> aNodeIndex);
    Const_Owned<gltf::Node> get(gltf::Index<gltf::Node> aNodeIndex) const;

    Owned<gltf::Scene> get(gltf::Index<gltf::Scene> aSceneIndex);
    Const_Owned<gltf::Scene> get(gltf::Index<gltf::Scene> aSceneIndex) const;

    Owned<gltf::Animation> get(gltf::Index<gltf::Animation> aAnimationIndex);
    Const_Owned<gltf::Animation> get(gltf::Index<gltf::Animation> aAnimationIndex) const;

    Owned<gltf::Material> get(gltf::Index<gltf::Material> aMaterialIndex);
    Const_Owned<gltf::Material> get(gltf::Index<gltf::Material> aMaterialIndex) const;

    filesystem::path getPathFor(gltf::Uri aFileUri) const;

private:
    filesystem::path mPath;

    std::optional<gltf::Index<gltf::Scene>> mDefaultScene;

    // the order in which they appear in Triangle.gltf sample
    std::vector<gltf::Scene> mScenes;
    std::vector<gltf::Node> mNodes;
    std::vector<gltf::Mesh> mMeshes;
    std::vector<gltf::Animation> mAnimations;
    std::vector<gltf::Buffer> mBuffers;
    std::vector<gltf::BufferView> mBufferViews;
    std::vector<gltf::Accessor> mAccessors;
    std::vector<gltf::Material> mMaterials;
};


} // namespace arte
} // namespace ad
