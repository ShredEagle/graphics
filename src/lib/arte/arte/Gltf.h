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
    using Index = std::vector<T_indexed>::size_type;

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


class Gltf
{
public:
    explicit Gltf(const filesystem::path & aGltfJson);

    std::optional<std::reference_wrapper<const gltf::Scene>> getDefaultScene() const;

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


} // namespace arte
} // namespace ad
