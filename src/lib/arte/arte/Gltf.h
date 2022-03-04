#pragma once


#include <platform/Filesystem.h>

#include <functional>
#include <map>
#include <optional>
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
        std::string string;
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
        std::string name;
        std::vector<Index<Node>> children;
        // TODO Handle non-mesh nodes
        Index<Mesh> mesh;
    };

    struct Scene
    {
        std::string name;
        std::vector<Index<Node>> nodes;
    };

    std::ostream & operator<<(std::ostream & aOut, const Scene & aScene);

} // namespace gltf


template <class T_element>
class Const_Owned
{
    friend class Gltf;

public:
    Const_Owned(const Gltf & aGltf, const T_element & aElement);

    template <class T_indexed>
    Const_Owned<T_indexed> get(gltf::Index<T_indexed> aIndex) const;

    operator const T_element &()
    { return mElement; }

    const T_element & elem()
    { return mElement; }

    // TODO make lazy
    template <class T_member>
    std::vector<Const_Owned<T_member>> iterate(std::vector<T_member> T_element::* aMemberVector)
    {
        std::vector<Const_Owned<T_member>> result;
        for (const T_member & element : mElement.*aMemberVector)
        {
            result.emplace_back(mOwningGltf, element);
        }
        return result;
    }

private:

    const Gltf & mOwningGltf;
    const T_element & mElement; 
};


class Gltf
{
public:
    explicit Gltf(const filesystem::path & aGltfJson);

    std::optional<std::reference_wrapper<const gltf::Scene>> getDefaultScene() const;

    Const_Owned<gltf::Accessor> get(gltf::Index<gltf::Accessor> aAccessorIndex) const;
    Const_Owned<gltf::Mesh> get(gltf::Index<gltf::Mesh> aMeshIndex) const;
    Const_Owned<gltf::Node> get(gltf::Index<gltf::Node> aNodeIndex) const;

private:
    std::optional<gltf::Index<gltf::Scene>> mDefaultScene;

    // the order in which they appear in Triangle.gltf sample
    std::vector<gltf::Scene> mScenes;
    std::vector<gltf::Node> mNodes;
    std::vector<gltf::Mesh> mMeshes;
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
