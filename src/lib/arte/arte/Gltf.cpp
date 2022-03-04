#include "Gltf.h"

#include "Logging.h"

#include "detail/Json.h"


namespace ad {
namespace arte {


using namespace gltf;


namespace {

constexpr const char * gTagAccessors        = "accessors";
constexpr const char * gTagAttributes       = "attributes";
constexpr const char * gTagBuffer           = "buffer";
constexpr const char * gTagBuffers          = "buffers";
constexpr const char * gTagBufferView       = "bufferView";
constexpr const char * gTagBufferViews      = "bufferViews";
constexpr const char * gTagByteLength       = "byteLength";
constexpr const char * gTagByteOffset       = "byteOffset";
constexpr const char * gTagByteStride       = "byteOffset";
constexpr const char * gTagChildren         = "children";
constexpr const char * gTagCount            = "count";
constexpr const char * gTagComponentType    = "componentType";
constexpr const char * gTagIndices          = "indices";
constexpr const char * gTagMesh             = "mesh";
constexpr const char * gTagMeshes           = "meshes";
constexpr const char * gTagMode             = "mode";
constexpr const char * gTagName             = "name";
constexpr const char * gTagNodes            = "nodes";
constexpr const char * gTagPrimitives       = "primitives";
constexpr const char * gTagScene            = "scene";
constexpr const char * gTagScenes           = "scenes";
constexpr const char * gTagTarget           = "target";
constexpr const char * gTagType             = "type";
constexpr const char * gTagUri              = "uri";

} // namespace anonymous


const std::array<std::string, 7> gElementTypeToString{
    "SCALAR",
    "VEC2",
    "VEC3",
    "VEC4",
    "MAT2",
    "MAT3",
    "MAT4",
};

const std::map<std::string, Accessor::ElementType> gStringToElementType{
    {"SCALAR", Accessor::ElementType::Scalar},
    {"VEC2",   Accessor::ElementType::Vec3},
    {"VEC3",   Accessor::ElementType::Vec3},
    {"VEC4",   Accessor::ElementType::Vec4},
    {"MAT2",   Accessor::ElementType::Mat2},
    {"MAT3",   Accessor::ElementType::Mat3},
    {"MAT4",   Accessor::ElementType::Mat4},
};


//
// Helpers
//
template <class T_index>
std::vector<T_index> makeIndicesVector(const Json & aArray)
{
    std::vector<T_index> result;
    // Is that a bug? the predicate seems strangej
    // Anyway, such syntax makes no sense
    //std::ranges::copy_if(aArray, std::back_inserter(result),
    //                     [](auto){return true;},
    //                     [](const auto & entry)->T_index{return entry.get<typename T_index::Value_t>();});
    for (typename T_index::Value_t index : aArray)
    {
        result.push_back(index);
    }
    return result;
}


// Note: Could not find a way to achieve it directly with json.value
template <class T_value, class T_tag>
std::optional<T_value> getOptional(Json aObject, T_tag && aTag)
{
    if (aObject.contains(std::forward<T_tag>(aTag)))
    {
        return T_value{aObject.at(std::forward<T_tag>(aTag))};
    }
    return std::nullopt;
}


template <class T_key>
Json getOptionalArray(const Json & aObject, T_key && aKey)
{
    return aObject.value(std::forward<T_key>(aKey), Json::array());
}


template <class T_object, class T_tag>
void populateVector(const Json & aJson, std::vector<T_object> & aVector, T_tag && aTag)
{
    aVector.reserve(aJson.at(std::forward<T_tag>(aTag)).size());
    for (auto object : aJson.at(aTag))
    {
        aVector.push_back(load<T_object>(object));
    }
}


//
// Element loaders
//
template <class T_object>
T_object load(const Json & aObjectJson);


template <>
Scene load(const Json & aJson)
{
    return{
        .name = aJson.value(gTagName, ""),
        .nodes = makeIndicesVector<Index<Node>>(getOptionalArray(aJson, gTagNodes)),
    };
}



template <>
Node load(const Json & aNodeObject)
{
    return Node{
        .name = aNodeObject.value(gTagName, ""),
        .children = makeIndicesVector<Index<Node>>(getOptionalArray(aNodeObject, gTagChildren)),
        .mesh = aNodeObject.at(gTagMesh), // TODO extend to other node types
    };
}


template <>
Primitive load(const Json & aPrimitiveObject)
{
    return Primitive{
        .mode = aPrimitiveObject.value<EnumType>(gTagMode, 4), // 4 is the default mode
        .attributes = [attributes = aPrimitiveObject.at(gTagAttributes)]()
            {
                std::map<std::string, Index<Accessor>> result;
                for (auto attribute : attributes.items())
                {
                    result.emplace(attribute.key(), attribute.value().get<Index<Accessor>::Value_t>());
                }
                return result;
            }(),
        .indices = getOptional<Index<Accessor>>(aPrimitiveObject, gTagIndices),
    };
}


template <>
Mesh load(const Json & aMeshObject)
{
    std::vector<Primitive> primitives;
    for (auto primitive : aMeshObject.at(gTagPrimitives))
    {
        primitives.push_back(load<Primitive>(primitive));
    }

    return Mesh{
        .primitives = std::move(primitives),
    };
}


template <>
Buffer load(const Json & aJson)
{
    return{
        .name = aJson.value(gTagName, ""),
        .uri = getOptional<Uri>(aJson, gTagUri),
        .byteLength = aJson.at(gTagByteLength),
    };
}


template <>
BufferView load(const Json & aJson)
{
    return{
        .name = aJson.value(gTagName, ""),
        .buffer = aJson.at(gTagBuffer),
        .byteOffset = aJson.value<std::size_t>(gTagByteOffset, 0),
        .byteLength = aJson.at(gTagByteLength),
        .byteStride = getOptional<std::size_t>(aJson, gTagByteStride),
        .target = getOptional<EnumType>(aJson, gTagTarget),
    };
}


template <>
Accessor load(const Json & aJson)
{
    return Accessor{
        .name = aJson.value(gTagName, ""),
        .bufferView = getOptional<Index<BufferView>>(aJson, gTagBufferView),
        .byteOffset = aJson.value<std::size_t>(gTagByteOffset, 0),
        .type = gStringToElementType.at(aJson.at(gTagType)),
        .componentType = aJson.at(gTagComponentType),
        .count = aJson.at(gTagCount),
    };
}


// 
// Output operators
// 


std::string prependNotEmpty(std::string_view aString, std::string aPrefix = " ")
{
    if (!aString.empty())
    {
        return aPrefix + std::string{aString};
    }
    else
    {
        return "";
    }
}


namespace gltf {

    std::ostream & operator<<(std::ostream & aOut, const Scene & aScene)
    {
        return aOut 
            << "<Scene>" << prependNotEmpty(aScene.name) 
            << " nodes: " << aScene.nodes;
    }

} // namespace gltf

//
// Gltf member functions
//
Gltf::Gltf(const filesystem::path & aGltfJson)
{
    std::ifstream jsonInput{aGltfJson.string()};
    Json json;
    jsonInput >> json;

    mDefaultScene = getOptional<Index<Scene>>(json, gTagScene);

    populateVector(json, mScenes, gTagScenes);
    populateVector(json, mNodes, gTagNodes);
    populateVector(json, mMeshes, gTagMeshes);
    populateVector(json, mBuffers, gTagBuffers);
    populateVector(json, mBufferViews, gTagBufferViews);
    populateVector(json, mAccessors, gTagAccessors);

    ADLOG(gMainLogger, info)("Loaded glTF file with {} scene(s), {} node(s), {} meshe(s), {} buffer(s).",
                             mScenes.size(), mNodes.size(), mMeshes.size(), mBuffers.size());
}


std::optional<std::reference_wrapper<const gltf::Scene>> Gltf::getDefaultScene() const
{
    if (mDefaultScene)
    {
        return std::cref(mScenes.at(*mDefaultScene));
    }
    return std::nullopt;
}


const Node & Gltf::get(Index<Node> aNodeIndex) const
{
    return mNodes.at(aNodeIndex);
}


const gltf::Mesh & Gltf::get(gltf::Index<gltf::Mesh> aMeshIndex) const
{
    return mMeshes.at(aMeshIndex);
}


} // namespace arte
} // namespace ad
