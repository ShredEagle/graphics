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

    namespace texture
    {

        struct Sampler
        {
            std::string name;
            std::optional<EnumType> magFilter;
            std::optional<EnumType> minFilter;
            EnumType wrapS{10497};
            EnumType wrapT{10497};
        };

        const Sampler gDefaultSampler;
    } // namespace texture

    struct BufferView;

    struct Image
    {
        enum class MimeType
        {
            ImageJpeg,
            ImagePng,
        };

        std::string name;
        std::variant<Uri, Index<BufferView>> dataSource; // correspond to either uri or bufferView.
        std::optional<MimeType> mimeType;
    };

    struct Texture
    {
        std::string name;
        std::optional<Index<Image>> source;
        std::optional<Index<texture::Sampler>> sampler;
    };

    struct TextureInfo
    {
        Index<Texture> index;
        unsigned int texCoord;
    };

    namespace material
    {
        struct PbrMetallicRoughness
        {
            math::hdr::Rgba<float> baseColorFactor{math::hdr::gWhite<float>};
            std::optional<TextureInfo> baseColorTexture;
        };

        constexpr material::PbrMetallicRoughness gDefaultPbr;
    } // namespace material

    struct Material
    {
        std::string name;
        std::optional<material::PbrMetallicRoughness> pbrMetallicRoughness;
    };

    const Material gDefaultMaterial;

    struct Accessor;

    struct Node;

    namespace animation {

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

    } // namespace animation

    struct Animation
    {
        std::string name;
        std::vector<animation::Channel> channels;
        std::vector<animation::Sampler> samplers;
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

        template <class T_stored>
        struct MinMax
        {
            using Store_t = std::vector<T_stored>; 

            Store_t min;
            Store_t max;
        };

        using Bounds_t = std::variant<MinMax<float>, MinMax<int>, MinMax<unsigned int>>;

        std::string name;
        // When absent, the accessor must be initialized with zeros 
        // (which can be overrided by sparse or extensions)
        std::optional<Index<BufferView>> bufferView;
        std::size_t byteOffset;
        ElementType type;
        EnumType componentType;
        bool normalized;
        std::size_t count;
        std::optional<Bounds_t> bounds;
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
std::string to_string(gltf::animation::Sampler::Interpolation aInterpolation);


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

    Owned<gltf::Image> get(gltf::Index<gltf::Image> aImageIndex);
    Const_Owned<gltf::Image> get(gltf::Index<gltf::Image> aImageIndex) const;

    Owned<gltf::Texture> get(gltf::Index<gltf::Texture> aTextureIndex);
    Const_Owned<gltf::Texture> get(gltf::Index<gltf::Texture> aTextureIndex) const;

    Owned<gltf::texture::Sampler> get(gltf::Index<gltf::texture::Sampler> aSamplerIndex);
    Const_Owned<gltf::texture::Sampler> get(gltf::Index<gltf::texture::Sampler> aSamplerIndex) const;

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
    std::vector<gltf::Image> mImages;
    std::vector<gltf::Texture> mTextures;
    std::vector<gltf::texture::Sampler> mSamplers;
};


} // namespace arte
} // namespace ad
