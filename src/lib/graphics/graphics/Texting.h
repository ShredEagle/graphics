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

    /// \param aCurveSubdivisions is the number of line segments used to approximate the curve.
    explicit Texting(filesystem::path aFontPath,
                     GLfloat aGlyphWorldHeight, 
                     GLfloat aScreenWorldHeight,
                     std::shared_ptr<AppInterface> aAppInterface);

    /// [aFirst, aLast[
    void loadGlyphs(arte::CharCode aFirst, arte::CharCode aLast);

    void updateInstances(gsl::span<const Instance> aInstances);
    void render() const;

    void setCameraTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation);
    void setProjectionTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation);

    template <class T_outputIterator>
    void prepareString(const std::string & aString, math::Position<2, GLfloat> aPenOrigin_w, T_outputIterator aOutput);

private:
    static constexpr GLsizei gVertexCount{4}; 
    static constexpr GLint gTextureUnit{0}; 

    VertexSpecification mVertexSpecification;
    VertexBufferObject mInstanceBuffer; // Will use the VAO from vertex spec
    Program mGpuProgram;

    arte::Freetype mFreetype;
    arte::FontFace mFontFace;
    math::Size<2, GLfloat> mPixelToWorld;
    detail::StaticGlyphCache mGlyphCache;
    GLsizei mInstanceCount{0};
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


template <class T_outputIterator>
void Texting::prepareString(const std::string & aString, math::Position<2, GLfloat> aPenOrigin_w, T_outputIterator aOutput)
{
    unsigned int previousIndex = 0;
    for (arte::CharCode codePoint : aString)
    {
        detail::RenderedGlyph rendered = mGlyphCache.at(codePoint);
        
        // Kerning
        if (previousIndex != 0)
        {
            Vec2<GLfloat> kerning = mFontFace.kern(previousIndex, rendered.freetypeIndex);
            aPenOrigin_w += kerning.cwMul(mPixelToWorld.as<math::Vec>());
        }
        previousIndex = rendered.freetypeIndex;

        *(aOutput++) = Texting::Instance{aPenOrigin_w, rendered};
        aPenOrigin_w += rendered.penAdvance.cwMul(mPixelToWorld.as<math::Vec>());
    }
}


} // namespace graphics
} // namespace ad
