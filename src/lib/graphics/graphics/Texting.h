#pragma once


#include "AppInterface.h"
#include "detail/GlyphUtilities.h"

#include <arte/Freetype.h>

#include <math/Homogeneous.h>
#include <math/Vector.h>

#include <platform/Filesystem.h>

#include <renderer/Shading.h>
#include <renderer/VertexSpecification.h>

#include <glad/glad.h>

#include <utf8.h> // utfcpp lib

#include <forward_list>
#include <functional>

namespace ad {
namespace graphics {


// TODO The pool should automatically handle freeing elements, probably by returning some smart handle
template <class T>
class Pool
{
private:
    void freeHandle(T * aResource)
    {
        mFreeList.push_front(aResource);
        --mLiveHandles;
    }


public:
    using Grower_t = std::function<T()>;
    using SmartHandle = std::unique_ptr<T, decltype(std::bind(&Pool::freeHandle,
                                                    std::declval<Pool*>(),
                                                    std::placeholders::_1))>;

    Pool(Grower_t aGrower) :
        mGrower{std::move(aGrower)}
    {}

    ~Pool()
    {
        // Otherwise, it means the Pool is destructed while handles are still out there.
        // Note: size() is not available on forward_list
        //assert(mFreeList.size() == mInstances.size());
        assert(mLiveHandles == 0);
    }

    SmartHandle getNext()
    {
        if (mFreeList.empty())
        {
            grow();
        }
        T * free = mFreeList.front();
        mFreeList.pop_front();
        ++mLiveHandles;
        return SmartHandle{free, std::bind(&Pool::freeHandle, this, std::placeholders::_1)};
    }

    void grow()
    {
        // TODO adapt logging
        //LOG(handy, debug)("Growing the pool");
        mInstances.push_front(mGrower());
        mFreeList.push_front(&mInstances.front());
    }

private:
    Grower_t mGrower;
    std::forward_list<T> mInstances;
    std::forward_list<T *> mFreeList;
    std::size_t mLiveHandles{0}; // only used for debug assertions, could be macro guarded
};


template <class T>
using Pooled = typename Pool<T>::SmartHandle;
    

template <class T_vertex>
VertexBufferObject loadUnattachedVertexBuffer(gsl::span<const T_vertex> aVertices,
                                              GLenum aHint = GL_STATIC_DRAW)
{
    VertexBufferObject vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, aVertices.size_bytes(), aVertices.data(), aHint);
    return vbo;
}


class Texting
{
public:
    struct Instance
    {
        Instance(math::Position<2, GLfloat> aPenOrigin_w, const detail::RenderedGlyph & aRendered);

        // TODO Ideally, this should be replaced by the local position of the glyph within the string, in pixel unit
        // this way, we do not have to keep mPixelToWorld value.
        math::Position<2, GLfloat> position_w; // position of the glyph instance in world coordinates.
        GLint offsetInTexture_p; // horizontal offset to the glyph in its texture.
        math::Size<2, GLfloat> boundingBox_p; // glyph bounding box in texture pixel coordinates;
        math::Vec<2, GLfloat> bearing_p;
    };

    /// \param aCurveSubdivisions is the number of line segments used to approximate the curve.
    explicit Texting(filesystem::path aFontPath,
                     GLfloat aGlyphWorldHeight, 
                     GLfloat aScreenWorldHeight,
                     std::shared_ptr<AppInterface> aAppInterface);

    /// [aFirst, aLast[
    void loadGlyphs(arte::CharCode aFirst, arte::CharCode aLast);

    /// \brief Replace the strings that will be renderer when calling `render()`.
    /// \note Use `prepareString()` to populate the mapping taken as argument.
    template <class T_mapping>
    void updateInstances(T_mapping aTextureMappedBuffers);

    /// \brief Render all strings loaded in the buffers via `updateInstances()`.
    void render() const;

    void setCameraTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation);
    void setProjectionTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation);

    /// \brief The interface to convert the user string to be rendered into a mapping accepted by `updateInstance()`.
    template <class T_mapping>
    void prepareString(const std::string & aString, math::Position<2, GLfloat> aPenOrigin_w, T_mapping & aOutputMap);

private:
    static constexpr GLsizei gVertexCount{4}; 
    static constexpr GLint gTextureUnit{0}; 

    struct PerTextureVao
    {
        VertexArrayObject vao; 
        VertexBufferObject instanceBuffer;
        Texture * texture{nullptr}; // Will point into a texture of the dynamic glyph atlas
        GLsizei instanceCount{0};
    };

    VertexBufferObject mQuadVbo; // will shared by all per-texture VAOs
    Pool<PerTextureVao> mVaoPool;
    std::vector<Pooled<PerTextureVao>> mPerTexture;
    Program mGpuProgram;

    arte::Freetype mFreetype;
    arte::FontFace mFontFace;
    math::Size<2, GLfloat> mPixelToWorld;
    // An alternative, with a single texture that has to be entirely pre-computed at instantiation.
    //detail::StaticGlyphCache mGlyphCache;
    detail::DynamicGlyphCache mGlyphCache;
};


//
// Implementations
//
inline Texting::Instance::Instance(math::Position<2, GLfloat> aPenOrigin_w, const detail::RenderedGlyph & aRendered) :
    position_w{aPenOrigin_w},
    offsetInTexture_p{aRendered.offsetInTexture},
    boundingBox_p{aRendered.boundingBox},
    bearing_p{aRendered.bearing}
{}


template <class T_mapping>
void Texting::updateInstances(T_mapping aTextureMappedBuffers)
{
    mPerTexture.clear();

    for (const auto & [texture, buffer] : aTextureMappedBuffers)
    {
        Pooled<PerTextureVao> perTexture = mVaoPool.getNext();
        perTexture->texture = texture;
        respecifyBuffer(perTexture->instanceBuffer, gsl::make_span(buffer));
        perTexture->instanceCount = (GLsizei)buffer.size();

        mPerTexture.push_back(std::move(perTexture));
    }

    // Note: Life would be simple if we had "ARB_base_instance" on all platforms (i.e. if macos)
    // We would create a single instancebuffer with all instances, and do sub-range of instances
    // per draw call. Instead, we are stuck with huge machinery to switch entire VAOs between draw calls.
    //mPerTexture.clear();
    //std::size_t count = 0;
    //for (const auto & [texture, buffer] : aTextureMappedBuffers)
    //{
    //    count += buffer.size();
    //}

    //glBindBuffer(GL_ARRAY_BUFFER, mInstanceBuffer);
    //// Orphan the previous buffer
    //glBufferData(GL_ARRAY_BUFFER, count * sizeof(Instance), NULL, GL_STATIC_DRAW);

    //GLsizei firstInstance = 0;
    //for (const auto & [texture, buffer] : aTextureMappedBuffers)
    //{
    //    mPerTexture.push_back(PerTextureInfo{
    //        texture,
    //        firstInstance,
    //        (GLsizei)buffer.size(),
    //    });

    //    glBufferSubData(GL_ARRAY_BUFFER,
    //                    firstInstance * sizeof(Instance),
    //                    buffer.size() * sizeof(Instance),
    //                    gsl::make_span(buffer).data());
    //    firstInstance += buffer.size();
    //}
}



template <class T_mapping>
void Texting::prepareString(const std::string & aString, math::Position<2, GLfloat> aPenOrigin_w, T_mapping & aOutputMap)
{
    unsigned int previousIndex = 0;
    for (std::string::const_iterator it = aString.begin();
         it != aString.end();
         /* in body */)
    {
        // Decode utf8 encoded string to individual Unicode code points
        arte::CharCode codePoint = utf8::next(it, aString.end());
        detail::RenderedGlyph rendered = mGlyphCache.at(codePoint, mFontFace);
        
        // Kerning
        if (previousIndex != 0)
        {
            Vec2<GLfloat> kerning = mFontFace.kern(previousIndex, rendered.freetypeIndex);
            aPenOrigin_w += kerning.cwMul(mPixelToWorld.as<math::Vec>());
        }
        previousIndex = rendered.freetypeIndex;

        aOutputMap[rendered.texture].push_back(Texting::Instance{aPenOrigin_w, rendered});
        aPenOrigin_w += rendered.penAdvance.cwMul(mPixelToWorld.as<math::Vec>());
    }
}


} // namespace graphics
} // namespace ad
