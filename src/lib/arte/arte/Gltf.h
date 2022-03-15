#pragma once


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

    template <class T_indexed>
    struct Index
    {
        using Value_t = std::vector<T_indexed>::size_type;

        Index(Value_t aValue) : 
            value{aValue}
        {}

        Index(const Index & aIndex) : 
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


// TODO Ad 2022/03/08 Embed the index of the element, to help debug printing.
template <class T_element>
class Const_Owned
{
    friend class Gltf;

public:
    Const_Owned(const Gltf & aGltf, const T_element & aElement);

    operator const T_element &() const
    { return mElement; }

    const T_element * operator->() const
    {
        return &mElement;
    }

    const T_element & operator*() const
    {
        return mElement;
    }

    template <class T_indexed>
    Const_Owned<T_indexed> get(gltf::Index<T_indexed> aIndex) const;

    template <class T_indexed>
    Const_Owned<T_indexed> get(gltf::Index<T_indexed> T_element::* aDataMember) const
    {
        return get(mElement.*aDataMember);
    }

    template <class T_indexed>
    Const_Owned<T_indexed> get(std::optional<gltf::Index<T_indexed>> T_element::* aDataMember) const
    {
        return get(*(mElement.*aDataMember));
    }

    template <class T_member>
    std::vector<Const_Owned<T_member>> 
    iterate(std::vector<gltf::Index<T_member>> T_element::* aMemberIndexVector) const
    {
        std::vector<Const_Owned<T_member>> result;
        for (gltf::Index<T_member> index : mElement.*aMemberIndexVector)
        {
            result.emplace_back(mOwningGltf.get(index));
        }
        return result;
    }

    // TODO make lazy iteration
    template <class T_member>
    std::vector<Const_Owned<T_member>> iterate(std::vector<T_member> T_element::* aMemberVector) const
    {
        std::vector<Const_Owned<T_member>> result;
        for (const T_member & element : mElement.*aMemberVector)
        {
            result.emplace_back(mOwningGltf, element);
        }
        return result;
    }

    filesystem::path getFilePath(gltf::Uri T_element::* aMemberUri) const
    {
        return mOwningGltf.getPathFor(mElement.*aMemberUri);
    }

    filesystem::path getFilePath(std::optional<gltf::Uri> T_element::* aMemberUri) const
    {
        return mOwningGltf.getPathFor(*(mElement.*aMemberUri));
    }


private:
    const Gltf & mOwningGltf;
    const T_element & mElement; 
};


class Gltf
{
public:
    explicit Gltf(const filesystem::path & aGltfJson);

    std::optional<Const_Owned<gltf::Scene>> getDefaultScene() const;

    std::vector<Const_Owned<gltf::Animation>> getAnimations() const;

    Const_Owned<gltf::Accessor> get(gltf::Index<gltf::Accessor> aAccessorIndex) const;
    Const_Owned<gltf::Buffer> get(gltf::Index<gltf::Buffer> aBufferIndex) const;
    Const_Owned<gltf::BufferView> get(gltf::Index<gltf::BufferView> aBufferViewIndex) const;
    Const_Owned<gltf::Mesh> get(gltf::Index<gltf::Mesh> aMeshIndex) const;
    Const_Owned<gltf::Node> get(gltf::Index<gltf::Node> aNodeIndex) const;
    Const_Owned<gltf::Scene> get(gltf::Index<gltf::Scene> aSceneIndex) const;
    Const_Owned<gltf::Animation> get(gltf::Index<gltf::Animation> aAnimationIndex) const;

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
};


//
// Implementations
//
namespace gltf 
{


    template <class T_indexed>
    std::ostream & operator<<(std::ostream & aOut, const std::vector<T_indexed> & aIndexVector)
    {
        auto begin = std::begin(aIndexVector);
        auto end = std::end(aIndexVector);

        aOut << "[";
        if (begin != end)
        {
            aOut << *begin;
            ++begin;
        }
        for (; begin != end; ++begin)
        {
            aOut << ", " << *begin;
        }
        return aOut << "]";
    }

     
} // namespace gltf


template <class T_element>
    Const_Owned<T_element>::Const_Owned(const Gltf & aGltf, const T_element & aElement) :
    mOwningGltf{aGltf},
    mElement{aElement}
{}


template <class T_element>
template <class T_indexed>
Const_Owned<T_indexed> Const_Owned<T_element>::get(gltf::Index<T_indexed> aIndex) const
{
    return mOwningGltf.get(aIndex);
}


} // namespace arte
} // namespace ad
