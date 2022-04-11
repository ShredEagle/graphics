#include "Mesh.h"

#include "DataLayout.h"
#include "LoadBuffer.h"
#include "Logging.h"

#include <renderer/GL_Loader.h>


namespace ad {


using namespace arte;


namespace gltfviewer {


/// \brief Maps semantic to vertex attribute indices that will be used in shaders.
const std::map<std::string /*semantic*/, GLuint /*vertex attribute index*/> gSemanticToAttribute{
    {"POSITION", 0},
    {"NORMAL", 1},
    {"TEXCOORD_0", 2}, // TODO Use the texCoord from TextureInfo
    {"COLOR_0", 3},
    {"JOINTS_0", 4},
    {"WEIGHTS_0", 5},
};


/// \brief First attribute index available for per-instance attributes.
constexpr GLuint gInstanceAttributeIndex = 8;

//
// Helper functions
//
template <class T_buffer>
constexpr GLenum associateTarget()
{
    if constexpr(std::is_same_v<T_buffer, graphics::VertexBufferObject>)
    {
        return(GL_ARRAY_BUFFER);
    }
    else if constexpr(std::is_same_v<T_buffer, graphics::IndexBufferObject>)
    {
        return(GL_ELEMENT_ARRAY_BUFFER);
    }
}


template <class T_buffer>
T_buffer prepareBuffer_impl(Const_Owned<gltf::Accessor> aAccessor)
{
    T_buffer buffer;
    auto bufferView = checkedBufferView(aAccessor);

    GLenum target = [&]()
    {
        if (!bufferView->target)
        {
            GLenum infered = associateTarget<T_buffer>();
            ADLOG(gPrepareLogger, warn)
                 ("Buffer view #{} does not have target defined. Infering {}.",
                  bufferView.id(), infered);
            return infered;
        }
        else
        {
            assert(*bufferView->target == associateTarget<T_buffer>());
            return *bufferView->target;
        }
    }();

    glBindBuffer(target, buffer);
    glBufferData(target,
                 bufferView->byteLength,
                 // TODO might be even better to only load in main memory the part of the buffer starting
                 // at bufferView->byteOffset (and also limit the length there, actually).
                 loadBufferData(aAccessor).data() 
                    + bufferView->byteOffset,
                 GL_STATIC_DRAW);
    glBindBuffer(target, 0);

    ADLOG(gPrepareLogger, debug)
         ("Loaded {} bytes in target {}, offset in source buffer is {} bytes.",
          bufferView->byteLength,
          target,
          bufferView->byteOffset);

    return buffer;
}


template <class T_component>
void outputElements(std::ostream & aOut,
                    std::span<T_component> aData,
                    std::size_t aElementCount,
                    VertexAttributeLayout aLayout,
                    std::size_t aComponentStride)
{
    std::size_t accessId = 0;
    for (std::size_t elementId = 0; elementId != aElementCount; ++elementId)
    {
        aOut << "{";
        // All element types have at least 1 component.
        aOut << aData[accessId];
        for (std::size_t componentId = 1; componentId != aLayout.totalComponents(); ++componentId)
        {
            aOut << ", " << aData[accessId + componentId];
        }
        aOut << "}, ";

        accessId += aComponentStride;
    }
}


template <class T_component>
void analyze_impl(Const_Owned<gltf::Accessor> aAccessor,
                  const std::vector<std::byte> & aBytes)
{
    auto bufferView = checkedBufferView(aAccessor);
    VertexAttributeLayout layout = gElementTypeToLayout.at(aAccessor->type);

    // If there is no explicit stride, the vertex attribute elements are tightly packed
    // i.e. the stride, in term of components, is the number of components in one element.
    std::size_t componentStride = layout.totalComponents();
    if (bufferView->byteStride)
    {
        auto stride = *bufferView->byteStride;
        assert(stride % sizeof(T_component) == 0);
        componentStride = stride / sizeof(T_component);
    }

    std::span<const T_component> span{
        reinterpret_cast<const T_component *>(aBytes.data() + bufferView->byteOffset + aAccessor->byteOffset), 
        // All the components, but not more (i.e. no "stride padding" after the last component)
        componentStride * (aAccessor->count - 1) + layout.totalComponents()
    };

    std::ostringstream oss;
    outputElements(oss, span, aAccessor->count, layout, componentStride);
    ADLOG(gPrepareLogger, debug)("Accessor content:\n{}", oss.str());
}


void analyzeAccessor(Const_Owned<gltf::Accessor> aAccessor)
{
    std::vector<std::byte> bytes = loadBufferData(aAccessor);

    switch(aAccessor->componentType)
    {
    default:
        ADLOG(gPrepareLogger, error)
             ("Analysis not available for component type {}.", aAccessor->componentType);
        return;
    case GL_UNSIGNED_SHORT:
    {
        analyze_impl<GLshort>(aAccessor, bytes);
        break;
    }
    case GL_FLOAT:
    {
        analyze_impl<GLfloat>(aAccessor, bytes);
        break;
    }
    }
}

//
// Loaded buffers types
//
Indices::Indices(Const_Owned<gltf::Accessor> aAccessor) :
    componentType{aAccessor->componentType},
    byteOffset{aAccessor->byteOffset},
    ibo{prepareBuffer_impl<graphics::IndexBufferObject>(aAccessor)}
{}


void InstanceList::update(std::span<Instance> aInstances)
{
    {
        graphics::bind_guard boundBuffer{mVbo};
        // Orphan the previous buffer, if any
        glBufferData(GL_ARRAY_BUFFER, aInstances.size_bytes(), NULL, GL_STREAM_DRAW);
        // Copy value to new buffer
        glBufferSubData(GL_ARRAY_BUFFER, 0, aInstances.size_bytes(), aInstances.data());
    }
    mInstanceCount = aInstances.size();
}


std::shared_ptr<graphics::Texture> loadGlTexture(arte::Image<math::sdr::Rgba> aTextureData, GLint aMipMapLevels)
{
    auto result = std::make_shared<graphics::Texture>(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, *result);

    // Allocate texture storage
    glTexStorage2D(
        GL_TEXTURE_2D,
        aMipMapLevels, 
        GL_RGBA8, // TODO should it be SRGB8_ALPHA8?
        aTextureData.width(),
        aTextureData.height());

    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0, 0,
        aTextureData.width(), aTextureData.height(),
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        aTextureData.data()
    );

    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    return result;
}


std::shared_ptr<graphics::Texture> Material::DefaultTexture()
{
    static std::shared_ptr<graphics::Texture> defaultTexture = []()
    {
        arte::Image<math::sdr::Rgba> whitePixel{{1, 1}, math::sdr::gWhite};
        return loadGlTexture(whitePixel, 1);
    }();
    return defaultTexture;
}


Material::Material(arte::Const_Owned<arte::gltf::Material> aMaterial) :
    baseColorFactor{GetPbr(aMaterial).baseColorFactor},
    alphaMode{aMaterial->alphaMode},
    doubleSided{aMaterial->doubleSided}
{
    gltf::material::PbrMetallicRoughness pbr = GetPbr(aMaterial);
    if(pbr.baseColorTexture)
    {
        gltf::TextureInfo info = *pbr.baseColorTexture;
        baseColorTexture = prepare(aMaterial.get<gltf::Texture>(info.index));
    }
    else
    {
        baseColorTexture = DefaultTexture();
    }
}


arte::gltf::material::PbrMetallicRoughness 
Material::GetPbr(arte::Const_Owned<arte::gltf::Material> aMaterial)
{
    return aMaterial->pbrMetallicRoughness.value_or(gltf::material::gDefaultPbr);
}


MeshPrimitive::BufferId::BufferId(arte::Const_Owned<arte::gltf::BufferView> aBufferView,
                                  arte::Const_Owned<arte::gltf::Accessor> aAccessor) :
    mBufferView{aBufferView.id()}
{
    // Only if the accessor is sparse should its index be used to differentiate buffers.
    if(aAccessor->sparse)
    {
        mAccessor = aAccessor.id();
    }
}


bool MeshPrimitive::BufferId::operator<(const BufferId & aRhs) const
{
    return mBufferView < aRhs.mBufferView
        || (mBufferView == aRhs.mBufferView && mAccessor < aRhs.mAccessor);
}


const ViewerVertexBuffer & MeshPrimitive::prepareVertexBuffer(Const_Owned<gltf::Accessor> aAccessor)
{
    auto bufferView = checkedBufferView(aAccessor);
    if (auto found = vbos.find(BufferId{bufferView, aAccessor});
        found != vbos.end())
    {
        return found->second;
    }
    else
    {
        auto vertexBuffer = prepareBuffer_impl<graphics::VertexBufferObject>(aAccessor);
        auto inserted = 
            vbos.emplace(BufferId{bufferView, aAccessor},
                         ViewerVertexBuffer{
                            std::move(vertexBuffer), 
                            bufferView->byteStride ? (GLsizei)*bufferView->byteStride : 0});
        return inserted.first->second;
    }
}


MeshPrimitive::MeshPrimitive(Const_Owned<gltf::Primitive> aPrimitive) :
    drawMode{aPrimitive->mode},
    material{aPrimitive.value_or(&gltf::Primitive::material, gltf::gDefaultMaterial)}
{
    graphics::bind_guard boundVao{vao};

    for (const auto & [semantic, accessorIndex] : aPrimitive->attributes)
    {
        ADLOG(gPrepareLogger, debug)("Semantic '{}' is associated to accessor #{}", semantic, accessorIndex);
        Const_Owned<gltf::Accessor> accessor = aPrimitive.get(accessorIndex);

        // All accessors for a given primitive must have the same count.
        count = accessor->count;

        if (!accessor->bufferView)
        {
            // TODO Handle no buffer view (accessor initialized to zeros)
            ADLOG(gPrepareLogger, error)
                 ("Unsupported: accessor #{} does not have a buffer view.", accessorIndex);
            continue;
        }

        const ViewerVertexBuffer & vertexBuffer = prepareVertexBuffer(accessor);

        if (gDumpBuffersContent) analyzeAccessor(accessor);

        if (auto found = gSemanticToAttribute.find(semantic);
            found != gSemanticToAttribute.end())
        {
            VertexAttributeLayout layout = gElementTypeToLayout.at(accessor->type);
            if (layout.occupiedAttributes != 1)
            {
                throw std::logic_error{"Matrix attributes not implemented yet."};
            }

            glEnableVertexAttribArray(found->second);

            graphics::bind_guard boundBuffer{vertexBuffer.vbo};

            // The vertex attributes in the shader are float, so use glVertexAttribPointer.
            glVertexAttribPointer(found->second,
                                  layout.componentsPerAttribute,
                                  accessor->componentType,
                                  accessor->normalized,
                                  vertexBuffer.stride,
                                  // Note: The buffer view byte offset is directly taken into account when loading data with glBufferData().
                                  reinterpret_cast<void *>(accessor->byteOffset)
                                  );

            if (semantic == "POSITION")
            {
                if (!accessor->bounds)
                {
                    throw std::logic_error{"Position's accessor MUST have bounds."};
                }
                // By the spec, position MUST be a VEC3 of float.
                auto & bounds = std::get<gltf::Accessor::MinMax<float>>(*accessor->bounds);

                math::Position<3, GLfloat> min{bounds.min[0], bounds.min[1], bounds.min[2]};
                math::Position<3, GLfloat> max{bounds.max[0], bounds.max[1], bounds.max[2]};
                boundingBox = {
                    min,
                    (max - min).as<math::Size>(),
                };

                ADLOG(gPrepareLogger, debug)
                     ("Mesh primitive #{} has bounding box {}.", aPrimitive.id(), boundingBox);
            }

            providedAttributes.insert(found->second);

            ADLOG(gPrepareLogger, debug)
                 ("Attached semantic '{}' to vertex attribute {}."
                  " Source data elements have {} components of type {}."
                  " OpenGL buffer #{}, stride is {}, offset is {}.",
                  semantic, found->second,
                  layout.componentsPerAttribute, accessor->componentType,
                  vertexBuffer.vbo, vertexBuffer.stride, accessor->byteOffset);
        }
        else
        {
            ADLOG(gPrepareLogger, warn)("Semantic '{}' is ignored.", semantic);
        }
    }

    if (aPrimitive->indices)
    {
        auto indicesAccessor = aPrimitive.get(&gltf::Primitive::indices);
        indices = Indices{indicesAccessor};
        count = indicesAccessor->count;

        if (gDumpBuffersContent) analyzeAccessor(indicesAccessor);
    }
}


MeshPrimitive::MeshPrimitive(arte::Const_Owned<arte::gltf::Primitive> aPrimitive,
                             const InstanceList & aInstances) :
    MeshPrimitive{aPrimitive}
{
    associateInstanceBuffer(aInstances);
}


void MeshPrimitive::associateInstanceBuffer(const InstanceList & aInstances)
{
    graphics::bind_guard boundVao{vao};
    graphics::bind_guard boundBuffer{aInstances.mVbo};

    for(GLuint attributeOffset = 0; attributeOffset != 4; ++attributeOffset)
    {
        GLuint attributeIndex = gInstanceAttributeIndex + attributeOffset;

        glEnableVertexAttribArray(attributeIndex);
        // The vertex attributes in the shader are float, so use glVertexAttribPointer.
        glVertexAttribPointer(attributeIndex,
                              4,
                              GL_FLOAT,
                              GL_FALSE, //normalized
                              sizeof(decltype(InstanceList::Instance::aModelTransform)),
                              reinterpret_cast<void *>(sizeof(GLfloat) * 4 * attributeOffset));
        glVertexAttribDivisor(attributeIndex, 1);
    }
}


bool MeshPrimitive::providesColor() const
{
    return providedAttributes.contains(gSemanticToAttribute.at("COLOR_0"));
};


std::shared_ptr<graphics::Texture> prepare(arte::Const_Owned<arte::gltf::Texture> aTexture)
{
    // TODO How should this value be decided?
    constexpr GLint gMipMapLevels = 6;

    auto image = aTexture.get(&gltf::Texture::source);
    std::shared_ptr<graphics::Texture> result{loadGlTexture(loadImageData(image), gMipMapLevels)};
    graphics::bind_guard boundTexture{*result};

    // Sampling parameters
    {
        const gltf::texture::Sampler & sampler = aTexture->sampler ? 
            aTexture.get(&gltf::Texture::sampler)
            : gltf::texture::gDefaultSampler;

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler.wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler.wrapT);
        if (sampler.magFilter)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, *sampler.magFilter);
        }
        if (sampler.minFilter)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, *sampler.minFilter);
        }
    }

    return result;
}


Mesh prepare(arte::Const_Owned<arte::gltf::Mesh> aMesh)
{
    Mesh mesh;

    auto primitives = aMesh.iterate(&arte::gltf::Mesh::primitives);
    auto primitiveIt = primitives.begin();
    
    // Note: the first iteration is taken out of the loop
    // because we do not want to unite with the zero bounding box initially in mesh.
    mesh.primitives.emplace_back(*primitiveIt, mesh.gpuInstances);
    mesh.boundingBox = mesh.primitives.back().boundingBox;

    for (++primitiveIt; primitiveIt != primitives.end(); ++primitiveIt)     
    {
        mesh.primitives.emplace_back(*primitiveIt, mesh.gpuInstances);
        mesh.boundingBox.uniteAssign(mesh.primitives.back().boundingBox);
    }
    return mesh;
}


std::ostream & operator<<(std::ostream & aOut, const MeshPrimitive & aPrimitive)
{
    return aOut << "<gltfviewer::MeshPrimitive> " 
                << (aPrimitive.indices ? "indexed" : "non-indexed")
                << " with " << aPrimitive.vbos.size() << " vbos."
        ;
}


std::ostream & operator<<(std::ostream & aOut, const Mesh & aMesh)
{
    aOut << "<gltfviewer::Mesh> " 
         << " with " << aMesh.primitives.size() << " primitives:"
        ;

    for (const auto & primitive : aMesh.primitives)
    {
        aOut << "\n\t* " << primitive;
    }
    
    return aOut;
}


} // namespace gltfviewer
} // namespace ad
