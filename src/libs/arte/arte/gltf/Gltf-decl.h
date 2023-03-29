//
// Attention: clients should include Gltf.h
//
#pragma once


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

    struct Uri
    {
        enum class Type
        {
            Data,
            File,
        };

        Uri(std::string aString) :
            string{std::move(aString)}
        {
            type = (string.rfind("data:", 0) == 0) ? Type::Data : Type::File;
        }

        std::string string;
        Type type;
    };


    template <class T_indexed>
    struct Index
    {
        using Value_t = typename std::vector<T_indexed>::size_type;

        constexpr Index(Value_t aValue) :
            value{aValue}
        {}

        constexpr Index(const Index & aIndex) :
            Index{aIndex.value}
        {}

        Index & operator=(Value_t aValue)
        {
            value = aValue;
            return this;
        }

        operator Value_t() const
        { return value; }

        Value_t value;
    };

    template <class T_indexed>
    std::ostream & operator<<(std::ostream & aOut, const std::vector<T_indexed> & aIndexVector);


    struct Camera
    {
        enum class Type
        {
            Orthographic,
            Perspective,
        };

        struct Orthographic
        {
            float xmag;
            float ymag;
            float zfar;
            float znear;
        };

        struct Perspective
        {
            std::optional<float> aspectRatio;
            float yfov;
            std::optional<float> zfar;
            float znear;
        };

        std::string name;
        Type type;
        std::variant<Orthographic, Perspective> projection;
    };

    struct Accessor;
    struct Node;

    struct Skin
    {
        std::string name;
        std::optional<Index<Accessor>> inverseBindMatrices;
        std::optional<Index<Node>> skeleton;
        std::vector<Index<Node>> joints;
    };

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

    struct NormalTextureInfo
    {
        Index<Texture> index;
        unsigned int texCoord;
        float scale;
    };

    struct OcclusionTextureInfo
    {
        Index<Texture> index;
        unsigned int texCoord;
        float strength;
    };

    namespace material
    {
        struct PbrMetallicRoughness
        {
            math::hdr::Rgba<float> baseColorFactor{math::hdr::gWhite<float>};
            std::optional<TextureInfo> baseColorTexture;
            float metallicFactor{1.f};
            float roughnessFactor{1.f};
            std::optional<TextureInfo> metallicRoughnessTexture;
        };

        constexpr material::PbrMetallicRoughness gDefaultPbr;

    } // namespace material

    struct Material
    {
        enum class AlphaMode
        {
            Opaque,
            Mask,
            Blend,
        };

        std::string name;
        std::optional<material::PbrMetallicRoughness> pbrMetallicRoughness;
        std::optional<NormalTextureInfo> normalTexture;
        std::optional<OcclusionTextureInfo> occlusionTexture;
        AlphaMode alphaMode{AlphaMode::Opaque};
        std::optional<float> alphaCutoff;
        bool doubleSided{false};
    };

    const Material gDefaultMaterial;

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

    namespace accessor {
        struct Indices
        {
            Index<BufferView> bufferView;
            std::size_t byteOffset;
            EnumType componentType;
        };

        struct Values
        {
            Index<BufferView> bufferView;
            std::size_t byteOffset;
        };

        struct Sparse
        {
            std::size_t count;
            Indices indices;
            Values values;
        };

    } // namespace accessor

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
        std::optional<accessor::Sparse> sparse;
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
        std::string name;
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
        std::optional<Index<Camera>> camera;
        std::vector<Index<Node>> children;
        std::variant<Matrix, TRS> transformation;
        std::optional<Index<Mesh>> mesh;
        std::optional<Index<Skin>> skin;

        // Custom member, not part of the gltf content
        bool usedAsJoint{false};
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
std::string to_string(gltf::Camera::Type aCameraType);


// Forward declarations (Gltf.h takes care of including the actual definitions after this header)
template <class T_element>
class Owned;

template <class T_element>
class Const_Owned;


class Gltf
{
public:
    explicit Gltf(const filesystem::path & aGltfJson);

    std::optional<Owned<gltf::Scene>> getDefaultScene();
    std::optional<Const_Owned<gltf::Scene>> getDefaultScene() const;

    std::vector<Owned<gltf::Animation>> getAnimations();
    std::vector<Const_Owned<gltf::Animation>> getAnimations() const;

    std::vector<Owned<gltf::Mesh>> getMeshes();
    std::vector<Const_Owned<gltf::Mesh>> getMeshes() const;

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

    Owned<gltf::Skin> get(gltf::Index<gltf::Skin> aSkinIndex);
    Const_Owned<gltf::Skin> get(gltf::Index<gltf::Skin> aSkinIndex) const;

    Owned<gltf::Camera> get(gltf::Index<gltf::Camera> aCameraIndex);
    Const_Owned<gltf::Camera> get(gltf::Index<gltf::Camera> aCameraIndex) const;

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
    std::vector<gltf::Skin> mSkins;
    std::vector<gltf::Camera> mCameras;
};


} // namespace arte
} // namespace ad
