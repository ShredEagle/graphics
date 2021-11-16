#pragma once


#include "AppInterface.h"
#include "detail/GlyphUtilities.h"

#include <arte/Freetype.h>

#include <handy/Pool.h>

#include <math/Homogeneous.h>
#include <math/Vector.h>

#include <platform/Filesystem.h>

#include <renderer/Shading.h>
#include <renderer/VertexSpecification.h>

#include <glad/glad.h>

#include <map>


namespace ad {
namespace graphics {


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

    /// \brief A good type for the templated T_mapping (but not the only possiblity).
    using Mapping = std::map<Texture *, std::vector<Texting::Instance>>;

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

    void setCameraTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation);
    void setProjectionTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation);

    /// \brief The interface to convert the user string to be rendered into a mapping accepted by `updateInstance()`.
    template <class T_mapping>
    void prepareString(const std::string & aString, math::Position<2, GLfloat> aPenOrigin_w, T_mapping & aOutputMap);

    /// \brief Compute a string's bounding box in world space. 
    ///
    /// It is a "tight" bounding box.
    /// The bounding-box will include all glyphs pixels, but not necessarily the pen origin / the final pen position.
    /// It is probably not exact due to Freetype details (but it should be close enough for most applications).
    math::Rectangle<GLfloat> getStringBounds(const std::string & aString, math::Position<2, GLfloat> aPenOrigin_w);

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
    handy::Pool<PerTextureVao> mVaoPool;
    std::vector<handy::Pooled<PerTextureVao>> mPerTexture;
    Program mGpuProgram;

    arte::Freetype mFreetype;
    arte::FontFace mFontFace;
    // TODO mPixelToWorld should be removed, and all "string local" pen computation be done in pixel units.
    // This would imply there is a "per string" buffer attribute which is the whole string transformation.
    // By using different cache, it would notably mean one can animate strings without respecifying all glyphs each frame
    // (but only the local-to-world transforms).
    // Note: It might still be required in order to compute string bounding box in world space? (or it would just take the transformation)
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
    boundingBox_p{aRendered.controlBoxSize},
    bearing_p{aRendered.bearing}
{}


template <class T_mapping>
void Texting::updateInstances(T_mapping aTextureMappedBuffers)
{
    mPerTexture.clear();

    for (const auto & [texture, buffer] : aTextureMappedBuffers)
    {
        handy::Pooled<PerTextureVao> perTexture = mVaoPool.getNext();
        perTexture->texture = texture;
        respecifyBuffer(perTexture->instanceBuffer, gsl::make_span(buffer));
        perTexture->instanceCount = (GLsizei)buffer.size();

        mPerTexture.push_back(std::move(perTexture));
    }

    // Note: Life would be simple if we had "ARB_base_instance" on all platforms (i.e. if it was available on macos)
    // We would create a single instancebuffer with all instances, and do sub-range of instances
    // per draw call with `glDrawElementsInstancedBaseInstance`.
    // Instead, we are stuck with huge machinery to switch entire VAOs between draw calls.
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
    forEachGlyph(aString, aPenOrigin_w, mGlyphCache, mFontFace, mPixelToWorld,
        [&aOutputMap](const detail::RenderedGlyph & rendered, math::Position<2, GLfloat> penPosition_w)
        {
            aOutputMap[rendered.texture].push_back(Texting::Instance{penPosition_w, rendered});
        });
}


} // namespace graphics
} // namespace ad
