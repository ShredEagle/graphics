#include "Gltf.h"

#include "../Logging.h"

#include "../detail/Json.h"
#include "../detail/GltfJson.h"


namespace ad {
namespace arte {


using namespace gltf;


namespace {

constexpr const char * gTagAccessors            = "accessors";
constexpr const char * gTagAlphaMode            = "alphaMode";
constexpr const char * gTagAlphaCutoff          = "alphaCutoff";
constexpr const char * gTagAnimations           = "animations";
constexpr const char * gTagAspectRatio          = "aspectRatio";
constexpr const char * gTagAttributes           = "attributes";
constexpr const char * gTagBuffer               = "buffer";
constexpr const char * gTagBuffers              = "buffers";
constexpr const char * gTagBufferView           = "bufferView";
constexpr const char * gTagBufferViews          = "bufferViews";
constexpr const char * gTagByteLength           = "byteLength";
constexpr const char * gTagByteOffset           = "byteOffset";
constexpr const char * gTagByteStride           = "byteStride";
constexpr const char * gTagCamera               = "camera";
constexpr const char * gTagCameras              = "cameras";
constexpr const char * gTagChannels             = "channels";
constexpr const char * gTagChildren             = "children";
constexpr const char * gTagBaseColorFactor      = "baseColorFactor";
constexpr const char * gTagBaseColorTexture     = "baseColorTexture";
constexpr const char * gTagCount                = "count";
constexpr const char * gTagComponentType        = "componentType";
constexpr const char * gTagDoubleSided          = "doubleSided";
constexpr const char * gTagImages               = "images";
constexpr const char * gTagIndex                = "index";
constexpr const char * gTagIndices              = "indices";
constexpr const char * gTagInput                = "input";
constexpr const char * gTagInterpolation        = "interpolation";
constexpr const char * gTagInverseBindMatrices  = "inverseBindMatrices";
constexpr const char * gTagJoints               = "joints";
constexpr const char * gTagMagFilter            = "magFilter";
constexpr const char * gTagMaterial             = "material";
constexpr const char * gTagMaterials            = "materials";
constexpr const char * gTagMatrix               = "matrix";
constexpr const char * gTagMax                  = "max";
constexpr const char * gTagMesh                 = "mesh";
constexpr const char * gTagMeshes               = "meshes";
constexpr const char * gTagMetallicFactor       = "metallicFactor";
constexpr const char * gTagMetallicRoughnessTexture = "metallicRoughnessTexture";
constexpr const char * gTagMimeType             = "mimeType";
constexpr const char * gTagMin                  = "min";
constexpr const char * gTagMinFilter            = "minFilter";
constexpr const char * gTagMode                 = "mode";
constexpr const char * gTagName                 = "name";
constexpr const char * gTagNode                 = "node";
constexpr const char * gTagNodes                = "nodes";
constexpr const char * gTagNormalized           = "normalized";
constexpr const char * gTagOrthographic         = "orthographic";
constexpr const char * gTagOutput               = "output";
constexpr const char * gTagPath                 = "path";
constexpr const char * gTagPbrMetallicRoughness = "pbrMetallicRoughness";
constexpr const char * gTagPerspective          = "perspective";
constexpr const char * gTagPrimitives           = "primitives";
constexpr const char * gTagRotation             = "rotation";
constexpr const char * gTagRoughnessFactor      = "roughnessFactor";
constexpr const char * gTagSampler              = "sampler";
constexpr const char * gTagSamplers             = "samplers";
constexpr const char * gTagScale                = "scale";
constexpr const char * gTagScene                = "scene";
constexpr const char * gTagScenes               = "scenes";
constexpr const char * gTagSkeleton             = "skeleton";
constexpr const char * gTagSkin                 = "skin";
constexpr const char * gTagSkins                = "skins";
constexpr const char * gTagSource               = "source";
constexpr const char * gTagSparse               = "sparse";
constexpr const char * gTagTarget               = "target";
constexpr const char * gTagTranslation          = "translation";
constexpr const char * gTagTexCoord             = "texCoord";
constexpr const char * gTagTextures             = "textures";
constexpr const char * gTagType                 = "type";
constexpr const char * gTagUri                  = "uri";
constexpr const char * gTagValues               = "values";
constexpr const char * gTagWrapS                = "wrapS";
constexpr const char * gTagWrapT                = "wrapT";
constexpr const char * gTagXMag                 = "xmag";
constexpr const char * gTagYFov                 = "yfov";
constexpr const char * gTagYMag                 = "ymag";
constexpr const char * gTagZFar                 = "zfar";
constexpr const char * gTagZNear                = "znear";

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

const std::map<std::string, animation::Target::Path> gStringToTargetPath{
    {"translation", animation::Target::Path::Translation},
    {"rotation",    animation::Target::Path::Rotation},
    {"scale",       animation::Target::Path::Scale},
    {"weights",     animation::Target::Path::Weights},
};


const std::array<std::string, 3> gSamplerInterpolationToString{
    "LINEAR",
    "STEP",
    "CUBICSPLINE",
};


std::string to_string(animation::Sampler::Interpolation aInterpolation)
{
    std::size_t index = static_cast<std::size_t>(aInterpolation);
    assert(index < gSamplerInterpolationToString.size());
    return gSamplerInterpolationToString.at(index);
}

const std::map<std::string, animation::Sampler::Interpolation> gStringToSamplerInterpolation{
    {"LINEAR",      animation::Sampler::Interpolation::Linear},
    {"STEP",        animation::Sampler::Interpolation::Step},
    {"CUBICSPLINE", animation::Sampler::Interpolation::CubicSpline},
};


const std::array<std::string, 2> gMimeTypeToString{
    "image/jpeg",
    "image/png",
};

const std::map<std::string, Image::MimeType> gStringToMimeType{
    {"image/jpeg",  Image::MimeType::ImageJpeg},
    {"image/png",  Image::MimeType::ImagePng},
};


const std::map<std::string, Material::AlphaMode> gStringToAlphaMode{
    {"OPAQUE",  Material::AlphaMode::Opaque},
    {"MASK",    Material::AlphaMode::Mask},
    {"BLEND",   Material::AlphaMode::Blend},
};


const std::map<std::string, gltf::Camera::Type> gStringToCameraType{
    {"orthographic",  Camera::Type::Orthographic},
    {"perspective",  Camera::Type::Perspective},
};

const std::array<std::string, 2> gCameraTypeToString{
    "orthographic",
    "perspective",
};

std::string to_string(gltf::Camera::Type aCameraType)
{
    std::size_t index = static_cast<std::size_t>(aCameraType);
    assert(index < gCameraTypeToString.size());
    return gCameraTypeToString.at(index);
}


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


template <class T_value, class T_tag>
std::optional<T_value> loadOptional(Json aObject, T_tag && aTag)
{
    if (aObject.contains(std::forward<T_tag>(aTag)))
    {
        return load<T_value>(aObject.at(std::forward<T_tag>(aTag)));
    }
    return std::nullopt;
}



template <class T_key>
Json getOptionalArray(const Json & aObject, T_key && aKey)
{
    return aObject.value(std::forward<T_key>(aKey), Json::array());
}


template <class T_object, class T_tag, class ... VT_args>
void populateVector(const Json & aJson, std::vector<T_object> & aVector, T_tag && aTag, VT_args && ... vaArgs)
{
    aVector.reserve(aJson.at(std::forward<T_tag>(aTag)).size());
    for (auto object : aJson.at(aTag))
    {
        aVector.push_back(load<T_object>(object, std::forward<VT_args>(vaArgs)...));
    }
}


template <class T_object, class T_tag, class ... VT_args>
void populateVectorIfPresent(const Json & aJson, std::vector<T_object> & aVector, T_tag && aTag, VT_args && ... vaArgs)
{
    if(aJson.contains(aTag))
    {
        populateVector(aJson, aVector, aTag, std::forward<VT_args>(vaArgs)...);
    }
}


//
// Element loaders
//
template <class T_object, class ... VT_args>
T_object load(const Json & aObjectJson, VT_args && ... vaArgs);


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
        .camera = getOptional<Index<Camera>>(aNodeObject, gTagCamera),
        .children = makeIndicesVector<Index<Node>>(getOptionalArray(aNodeObject, gTagChildren)),
        .transformation = handleTransformation(aNodeObject),
        .mesh = getOptional<Index<Mesh>>(aNodeObject, gTagMesh),
        .skin = getOptional<Index<Skin>>(aNodeObject, gTagSkin),
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
        .material = getOptional<Index<Material>>(aPrimitiveObject, gTagMaterial),
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
accessor::Sparse load(const Json & aJson)
{
    auto indicesLoader = [](const Json & aJson) -> accessor::Indices
    {
        return {
            .bufferView = aJson.at(gTagBufferView).get<Index<BufferView>>(),
            .byteOffset = aJson.value<std::size_t>(gTagByteOffset, 0),
            .componentType = aJson.at(gTagComponentType),
        };
    };

    auto valuesLoader = [](const Json & aJson) -> accessor::Values
    {
        return {
            .bufferView = aJson.at(gTagBufferView).get<Index<BufferView>>(),
            .byteOffset = aJson.value<std::size_t>(gTagByteOffset, 0),
        };
    };

    return {
        .count = aJson.at(gTagCount),
        .indices = indicesLoader(aJson.at(gTagIndices)),
        .values = valuesLoader(aJson.at(gTagValues)),
    };
}


template <class T_stored>
Accessor::MinMax<T_stored> makeMinMax(const Json & aJson)
{
    return {
        .min = aJson.at(gTagMin).get<std::vector<T_stored>>(),
        .max = aJson.at(gTagMax).get<std::vector<T_stored>>(),
    };
}


template <>
Accessor load(const Json & aJson)
{
    Accessor result{
        .name = aJson.value(gTagName, ""),
        .bufferView = getOptional<Index<BufferView>>(aJson, gTagBufferView),
        .byteOffset = aJson.value<std::size_t>(gTagByteOffset, 0),
        .type = gStringToElementType.at(aJson.at(gTagType)),
        .componentType = aJson.at(gTagComponentType),
        .normalized = aJson.value(gTagNormalized, false),
        .count = aJson.at(gTagCount),
        .sparse = loadOptional<accessor::Sparse>(aJson, gTagSparse),
    };

    // Note: As arte is an upstream of renderer (which currently defines the gl loader includes)
    // we do not have an "easy" access to OpenGL symbols and enumerators from here.
    if (aJson.contains(gTagMax))
    {
        switch(result.componentType)
        {
        case 5120: // GL_BYTE
        case 5122: // GL_SHORT
            result.bounds = makeMinMax<int>(aJson);
            break;
        case 5121: // GL_UNSIGNED_BYTE
        case 5123: // GL_UNSIGNED_SHORT
        case 5125: // GL_UNSIGNED_INT
            result.bounds = makeMinMax<unsigned int>(aJson);
            break;
        case 5126: // GL_FLOAT
            result.bounds = makeMinMax<float>(aJson);
            break;
        }
    }

    return result;
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
animation::Channel load(const Json & aJson)
{
    Json target = aJson.at(gTagTarget);

    return {
        .sampler = aJson.at(gTagSampler).get<Index<animation::Sampler>>(),
        .target = {
            .node = getOptional<Index<Node>>(target, gTagNode),
            .path = gStringToTargetPath.at(target.at(gTagPath)),
        },
    };
}


template <>
animation::Sampler load(const Json & aJson)
{
    return {
        .input = aJson.at(gTagInput).get<Index<Accessor>>(),
        .interpolation = gStringToSamplerInterpolation.at(aJson.value(gTagInterpolation, "LINEAR")),
        .output = aJson.at(gTagOutput).get<Index<Accessor>>(),
    };
}


template <>
TextureInfo load(const Json & aJson)
{
    return {
        .index = aJson.at(gTagIndex).get<Index<Texture>>(),
        .texCoord = aJson.value<unsigned int>(gTagTexCoord, 0),
    };
}


template <>
material::PbrMetallicRoughness load(const Json & aJson)
{
    return {
        .baseColorFactor = 
            aJson.value<math::hdr::Rgba<float>>(gTagBaseColorFactor,
                                                material::gDefaultPbr.baseColorFactor),
        .baseColorTexture = loadOptional<TextureInfo>(aJson, gTagBaseColorTexture),
        .metallicFactor = aJson.value(gTagMetallicFactor, material::gDefaultPbr.metallicFactor),
        .roughnessFactor = aJson.value(gTagRoughnessFactor, material::gDefaultPbr.roughnessFactor),
        .metallicRoughnessTexture = loadOptional<TextureInfo>(aJson, gTagMetallicRoughnessTexture),
    };
}


template <>
Material load(const Json & aJson)
{
    Material result{
        .name = aJson.value(gTagName, ""),
        .pbrMetallicRoughness = 
            loadOptional<material::PbrMetallicRoughness>(aJson, gTagPbrMetallicRoughness),
        .alphaCutoff = getOptional<float>(aJson, gTagAlphaCutoff),
        .doubleSided = aJson.value(gTagDoubleSided, gDefaultMaterial.doubleSided),
    };

    if(aJson.contains(gTagAlphaMode))
    {
        result.alphaMode = gStringToAlphaMode.at(aJson.at(gTagAlphaMode).get<std::string>());
    }

    return result;
}   


template <>
Texture load(const Json & aJson)
{
    return {
        .name = aJson.value(gTagName, ""),
        .source = getOptional<Index<Image>>(aJson, gTagSource),
        .sampler = getOptional<Index<texture::Sampler>>(aJson, gTagSampler),
    };
}   


template <>
Image load(const Json & aJson)
{
    auto handleDataSource = [](const Json & aJson) -> std::variant<Uri, Index<BufferView>>
    {
        if (aJson.contains(gTagUri))
        {
            return Uri{aJson.at(gTagUri)};
        }
        else
        {
            return aJson.at(gTagBufferView).get<Index<BufferView>>();
        }
    };
    
    Image image{
        .name = aJson.value(gTagName, ""),
        .dataSource = handleDataSource(aJson),
    };

    if (aJson.contains(gTagMimeType))
    {
        image.mimeType = gStringToMimeType.at(aJson.at(gTagMimeType));
    }

    return image;
}   


template <>
texture::Sampler load(const Json & aJson)
{
    return {
        .name = aJson.value(gTagName, ""),
        .magFilter = getOptional<EnumType>(aJson, gTagMagFilter),
        .minFilter = getOptional<EnumType>(aJson, gTagMinFilter),
        .wrapS = aJson.value(gTagWrapS, texture::gDefaultSampler.wrapS),
        .wrapT = aJson.value(gTagWrapT, texture::gDefaultSampler.wrapT),
    };
}   


template <>
Skin load(const Json & aJson, Gltf & aGltf)
{
    Skin result{
        .name = aJson.value(gTagName, ""),
        .inverseBindMatrices = getOptional<Index<Accessor>>(aJson, gTagInverseBindMatrices),
        .skeleton = getOptional<Index<Node>>(aJson, gTagSkeleton),
        .joints = makeIndicesVector<Index<Node>>(getOptionalArray(aJson, gTagJoints)),
    };

    for (auto joint : result.joints)
    {
        aGltf.get(joint)->usedAsJoint = true;
    }

    return result;
}


template <>
Camera load(const Json & aJson)
{
    Camera result{
        .name = aJson.value(gTagName, ""),
        .type = gStringToCameraType.at(aJson.at(gTagType)),
    };

    switch(result.type)
    {
    case Camera::Type::Orthographic:
    {
        auto proj = aJson.at(gTagOrthographic);
        result.projection = Camera::Orthographic{
            .xmag = proj.at(gTagXMag),
            .ymag = proj.at(gTagYMag),
            .zfar = proj.at(gTagZFar),
            .znear = proj.at(gTagZNear),
        };
        break;
    }
    case Camera::Type::Perspective:
    {
        auto proj = aJson.at(gTagPerspective);
        result.projection = Camera::Perspective{
            .aspectRatio = getOptional<float>(proj, gTagAspectRatio),
            .yfov = proj.at(gTagYFov),
            .zfar = getOptional<float>(proj, gTagZFar),
            .znear = proj.at(gTagZNear),
        };
        break;
    }
    }

    return result;
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
    populateVectorIfPresent(json, mAnimations, gTagAnimations);
    populateVector(json, mBuffers, gTagBuffers);
    populateVector(json, mBufferViews, gTagBufferViews);
    populateVector(json, mAccessors, gTagAccessors);
    populateVectorIfPresent(json, mMaterials, gTagMaterials);
    populateVectorIfPresent(json, mImages, gTagImages);
    populateVectorIfPresent(json, mTextures, gTagTextures);
    populateVectorIfPresent(json, mSamplers, gTagSamplers);
    populateVectorIfPresent(json, mSkins, gTagSkins, *this);
    populateVectorIfPresent(json, mCameras, gTagCameras);

    ADLOG(gMainLogger, info)("Loaded glTF file with {} scene(s), {} node(s), {} meshe(s), {} material(s), {} animation(s), {} skin(s), {} camera(s), {} buffer(s).",
                             mScenes.size(), mNodes.size(), mMeshes.size(), mMaterials.size(), mAnimations.size(), mSkins.size(), mCameras.size(), mBuffers.size());
}


std::optional<Owned<gltf::Scene>> Gltf::getDefaultScene()
{
    if (mDefaultScene)
    {
        return get(*mDefaultScene);
    }
    return std::nullopt;
}

std::optional<Const_Owned<gltf::Scene>> Gltf::getDefaultScene() const
{
    if (mDefaultScene)
    {
        return get(*mDefaultScene);
    }
    return std::nullopt;
}


std::vector<Owned<gltf::Animation>> Gltf::getAnimations()
{
    std::vector<Owned<gltf::Animation>> result;
    for (std::size_t id = 0; id != mAnimations.size(); ++id)
    {
        result.emplace_back(*this, mAnimations[id], id);
    }
    return result;
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


Owned<gltf::Accessor> Gltf::get(gltf::Index<gltf::Accessor> aAccessorIndex)
{
    return {*this, mAccessors.at(aAccessorIndex), aAccessorIndex};
}

Const_Owned<gltf::Accessor> Gltf::get(gltf::Index<gltf::Accessor> aAccessorIndex) const
{
    return {*this, mAccessors.at(aAccessorIndex), aAccessorIndex};
}


Owned<gltf::Buffer> Gltf::get(gltf::Index<gltf::Buffer> aBufferIndex)
{
    return {*this, mBuffers.at(aBufferIndex), aBufferIndex};
}

Const_Owned<gltf::Buffer> Gltf::get(gltf::Index<gltf::Buffer> aBufferIndex) const
{
    return {*this, mBuffers.at(aBufferIndex), aBufferIndex};
}


Owned<gltf::BufferView> Gltf::get(gltf::Index<gltf::BufferView> aBufferViewIndex)
{
    return {*this, mBufferViews.at(aBufferViewIndex), aBufferViewIndex};
}

Const_Owned<gltf::BufferView> Gltf::get(gltf::Index<gltf::BufferView> aBufferViewIndex) const
{
    return {*this, mBufferViews.at(aBufferViewIndex), aBufferViewIndex};
}


Owned<gltf::Mesh> Gltf::get(gltf::Index<gltf::Mesh> aMeshIndex)
{
    return {*this, mMeshes.at(aMeshIndex), aMeshIndex};
}

Const_Owned<gltf::Mesh> Gltf::get(gltf::Index<gltf::Mesh> aMeshIndex) const
{
    return {*this, mMeshes.at(aMeshIndex), aMeshIndex};
}


Owned<Node> Gltf::get(gltf::Index<gltf::Node> aNodeIndex)
{
    return {*this, mNodes.at(aNodeIndex), aNodeIndex};
}

Const_Owned<Node> Gltf::get(gltf::Index<gltf::Node> aNodeIndex) const
{
    return {*this, mNodes.at(aNodeIndex), aNodeIndex};
}


Owned<gltf::Scene> Gltf::get(gltf::Index<gltf::Scene> aSceneIndex)
{
    return {*this, mScenes.at(aSceneIndex), aSceneIndex};
}

Const_Owned<gltf::Scene> Gltf::get(gltf::Index<gltf::Scene> aSceneIndex) const
{
    return {*this, mScenes.at(aSceneIndex), aSceneIndex};
}


Owned<gltf::Animation> Gltf::get(gltf::Index<gltf::Animation> aAnimationIndex)
{
    return {*this, mAnimations.at(aAnimationIndex), aAnimationIndex};
}

Const_Owned<gltf::Animation> Gltf::get(gltf::Index<gltf::Animation> aAnimationIndex) const
{
    return {*this, mAnimations.at(aAnimationIndex), aAnimationIndex};
}


Owned<gltf::Material> Gltf::get(gltf::Index<gltf::Material> aMaterialIndex)
{
    return {*this, mMaterials.at(aMaterialIndex), aMaterialIndex};
}

Const_Owned<gltf::Material> Gltf::get(gltf::Index<gltf::Material> aMaterialIndex) const
{
    return {*this, mMaterials.at(aMaterialIndex), aMaterialIndex};
}


Owned<gltf::Image> Gltf::get(gltf::Index<gltf::Image> aImageIndex)
{
    return {*this, mImages.at(aImageIndex), aImageIndex};
}

Const_Owned<gltf::Image> Gltf::get(gltf::Index<gltf::Image> aImageIndex) const
{
    return {*this, mImages.at(aImageIndex), aImageIndex};
}


Owned<gltf::Texture> Gltf::get(gltf::Index<gltf::Texture> aTextureIndex)
{
    return {*this, mTextures.at(aTextureIndex), aTextureIndex};
}

Const_Owned<gltf::Texture> Gltf::get(gltf::Index<gltf::Texture> aTextureIndex) const
{
    return {*this, mTextures.at(aTextureIndex), aTextureIndex};
}


Owned<gltf::texture::Sampler> Gltf::get(gltf::Index<gltf::texture::Sampler> aSamplerIndex)
{
    return {*this, mSamplers.at(aSamplerIndex), aSamplerIndex};
}

Const_Owned<gltf::texture::Sampler> Gltf::get(gltf::Index<gltf::texture::Sampler> aSamplerIndex) const
{
    return {*this, mSamplers.at(aSamplerIndex), aSamplerIndex};
}


Owned<gltf::Skin> Gltf::get(gltf::Index<gltf::Skin> aSkinIndex)
{
    return {*this, mSkins.at(aSkinIndex), aSkinIndex};
}

Const_Owned<gltf::Skin> Gltf::get(gltf::Index<gltf::Skin> aSkinIndex) const
{
    return {*this, mSkins.at(aSkinIndex), aSkinIndex};
}


Owned<gltf::Camera> Gltf::get(gltf::Index<gltf::Camera> aCameraIndex)
{
    return {*this, mCameras.at(aCameraIndex), aCameraIndex};
}

Const_Owned<gltf::Camera> Gltf::get(gltf::Index<gltf::Camera> aCameraIndex) const
{
    return {*this, mCameras.at(aCameraIndex), aCameraIndex};
}


filesystem::path Gltf::getPathFor(gltf::Uri aFileUri) const
{
    assert(aFileUri.type == gltf::Uri::Type::File);
    return mPath.parent_path() / aFileUri.string;
}


} // namespace arte
} // namespace ad
