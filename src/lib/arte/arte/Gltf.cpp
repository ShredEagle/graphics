#include "Gltf.h"

#include "Logging.h"

#include "detail/Json.h"
#include "detail/GltfJson.h"


namespace ad {
namespace arte {


using namespace gltf;


namespace {

constexpr const char * gTagAccessors        = "accessors";
constexpr const char * gTagAnimations       = "animations";
constexpr const char * gTagAttributes       = "attributes";
constexpr const char * gTagBuffer           = "buffer";
constexpr const char * gTagBuffers          = "buffers";
constexpr const char * gTagBufferView       = "bufferView";
constexpr const char * gTagBufferViews      = "bufferViews";
constexpr const char * gTagByteLength       = "byteLength";
constexpr const char * gTagByteOffset       = "byteOffset";
constexpr const char * gTagByteStride       = "byteStride";
constexpr const char * gTagChannels         = "channels";
constexpr const char * gTagChildren         = "children";
constexpr const char * gTagCount            = "count";
constexpr const char * gTagComponentType    = "componentType";
constexpr const char * gTagIndices          = "indices";
constexpr const char * gTagInput            = "input";
constexpr const char * gTagInterpolation    = "interpolation";
constexpr const char * gTagMatrix           = "matrix";
constexpr const char * gTagMesh             = "mesh";
constexpr const char * gTagMeshes           = "meshes";
constexpr const char * gTagMode             = "mode";
constexpr const char * gTagName             = "name";
constexpr const char * gTagNode             = "node";
constexpr const char * gTagNodes            = "nodes";
constexpr const char * gTagNormalized       = "normalized";
constexpr const char * gTagOutput           = "output";
constexpr const char * gTagPath             = "path";
constexpr const char * gTagPrimitives       = "primitives";
constexpr const char * gTagRotation         = "rotation";
constexpr const char * gTagSampler          = "sampler";
constexpr const char * gTagSamplers         = "samplers";
constexpr const char * gTagScale            = "scale";
constexpr const char * gTagScene            = "scene";
constexpr const char * gTagScenes           = "scenes";
constexpr const char * gTagTarget           = "target";
constexpr const char * gTagTranslation      = "translation";
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


std::string to_string(Accessor::ElementType aElementType)
{
    std::size_t index = static_cast<std::size_t>(aElementType);
    assert(index < gElementTypeToString.size());
    return gElementTypeToString.at(index);
}

const std::map<std::string, Accessor::ElementType> gStringToElementType{
    {"SCALAR", Accessor::ElementType::Scalar},
    {"VEC2",   Accessor::ElementType::Vec2},
    {"VEC3",   Accessor::ElementType::Vec3},
    {"VEC4",   Accessor::ElementType::Vec4},
    {"MAT2",   Accessor::ElementType::Mat2},
    {"MAT3",   Accessor::ElementType::Mat3},
    {"MAT4",   Accessor::ElementType::Mat4},
};


const std::array<std::string, 4> gTargetPathToString{
    "translation",
    "rotation",
    "scale",
    "weights",
};

const std::map<std::string, Target::Path> gStringToTargetPath{
    {"translation", Target::Path::Translation},
    {"rotation",    Target::Path::Rotation},
    {"scale",       Target::Path::Scale},
    {"weights",     Target::Path::Weights},
};


const std::array<std::string, 3> gSamplerInterpolationToString{
    "LINEAR",
    "STEP",
    "CUBICSPLINE",
};


std::string to_string(Sampler::Interpolation aInterpolation)
{
    std::size_t index = static_cast<std::size_t>(aInterpolation);
    assert(index < gSamplerInterpolationToString.size());
    return gSamplerInterpolationToString.at(index);
}

const std::map<std::string, Sampler::Interpolation> gStringToSamplerInterpolation{
    {"LINEAR",      Sampler::Interpolation::Linear},
    {"STEP",        Sampler::Interpolation::Step},
    {"CUBICSPLINE", Sampler::Interpolation::CubicSpline},
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
    auto handleTransformation = [](const Json & aNodeObject) -> std::variant<Node::Matrix, Node::TRS>
    {
        if (aNodeObject.contains(gTagRotation)
            || aNodeObject.contains(gTagScale)
            || aNodeObject.contains(gTagTranslation))
        {
            return Node::TRS{
                .translation = aNodeObject.value(gTagTranslation, math::Vec<3, float>{0.f, 0.f, 0.f}),
                .rotation = aNodeObject.value(gTagRotation, math::Quaternion<float>::Identity()),
                .scale = aNodeObject.value(gTagScale, math::Vec<3, float>{1.f, 1.f, 1.f}),
            };
        }
        else
        {
            return aNodeObject.value(gTagMatrix, math::AffineMatrix<4, float>::Identity());
        }
    };

    return Node{
        .name = aNodeObject.value(gTagName, ""),
        .children = makeIndicesVector<Index<Node>>(getOptionalArray(aNodeObject, gTagChildren)),
        .transformation = handleTransformation(aNodeObject),
        .mesh = getOptional<Index<Mesh>>(aNodeObject, gTagMesh),
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
        .buffer = aJson.at(gTagBuffer).get<Index<Buffer>::Value_t>(),
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
        .normalized = aJson.value(gTagNormalized, false),
        .count = aJson.at(gTagCount),
    };
}


template <>
Animation load(const Json & aJson)
{
    Animation animation{
        .name = aJson.value(gTagName, ""),
    };

    populateVector(aJson, animation.channels, gTagChannels);
    populateVector(aJson, animation.samplers, gTagSamplers);

    return animation;
}


template <>
Channel load(const Json & aJson)
{
    Json target = aJson.at(gTagTarget);

    return {
        .sampler = aJson.at(gTagSampler).get<Index<Sampler>>(),
        .target = {
            .node = getOptional<Index<Node>>(target, gTagNode),
            .path = gStringToTargetPath.at(target.at(gTagPath)),
        },
    };
}


template <>
Sampler load(const Json & aJson)
{
    return {
        .input = aJson.at(gTagInput).get<Index<Accessor>>(),
        .interpolation = gStringToSamplerInterpolation.at(aJson.value(gTagInterpolation, "LINEAR")),
        .output = aJson.at(gTagOutput).get<Index<Accessor>>(),
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
Gltf::Gltf(const filesystem::path & aGltfJson) :
    mPath{aGltfJson}
{
    std::ifstream jsonInput{aGltfJson.string()};
    Json json;
    jsonInput >> json;

    mDefaultScene = getOptional<Index<Scene>>(json, gTagScene);

    populateVector(json, mScenes, gTagScenes);
    populateVector(json, mNodes, gTagNodes);
    populateVector(json, mMeshes, gTagMeshes);
    if(json.contains(gTagAnimations))
    {
        populateVector(json, mAnimations, gTagAnimations);
    }
    populateVector(json, mBuffers, gTagBuffers);
    populateVector(json, mBufferViews, gTagBufferViews);
    populateVector(json, mAccessors, gTagAccessors);

    ADLOG(gMainLogger, info)("Loaded glTF file with {} scene(s), {} node(s), {} meshe(s), {} animation(s), {} buffer(s).",
                             mScenes.size(), mNodes.size(), mMeshes.size(), mAnimations.size(), mBuffers.size());
}


std::optional<Const_Owned<gltf::Scene>> Gltf::getDefaultScene() const
{
    if (mDefaultScene)
    {
        return get(*mDefaultScene);
    }
    return std::nullopt;
}


std::vector<Const_Owned<gltf::Animation>> Gltf::getAnimations() const
{
    std::vector<Const_Owned<gltf::Animation>> result;
    for (std::size_t id = 0; id != mAnimations.size(); ++id)
    {
        result.emplace_back(*this, mAnimations[id], id);
    }
    return result;
}


Const_Owned<gltf::Accessor> Gltf::get(gltf::Index<gltf::Accessor> aAccessorIndex) const
{
    return {*this, mAccessors.at(aAccessorIndex), aAccessorIndex};
}


Const_Owned<gltf::Buffer> Gltf::get(gltf::Index<gltf::Buffer> aBufferIndex) const
{
    return {*this, mBuffers.at(aBufferIndex), aBufferIndex};
}


Const_Owned<gltf::BufferView> Gltf::get(gltf::Index<gltf::BufferView> aBufferViewIndex) const
{
    return {*this, mBufferViews.at(aBufferViewIndex), aBufferViewIndex};
}


Const_Owned<gltf::Mesh> Gltf::get(gltf::Index<gltf::Mesh> aMeshIndex) const
{
    return {*this, mMeshes.at(aMeshIndex), aMeshIndex};
}


Const_Owned<Node> Gltf::get(gltf::Index<gltf::Node> aNodeIndex) const
{
    return {*this, mNodes.at(aNodeIndex), aNodeIndex};
}


Const_Owned<gltf::Scene> Gltf::get(gltf::Index<gltf::Scene> aSceneIndex) const
{
    return {*this, mScenes.at(aSceneIndex), aSceneIndex};
}


Const_Owned<gltf::Animation> Gltf::get(gltf::Index<gltf::Animation> aAnimationIndex) const
{
    return {*this, mAnimations.at(aAnimationIndex), aAnimationIndex};
}


filesystem::path Gltf::getPathFor(gltf::Uri aFileUri) const
{
    assert(aFileUri.type == gltf::Uri::Type::File);
    return mPath.parent_path() / aFileUri.string;
}


} // namespace arte
} // namespace ad
