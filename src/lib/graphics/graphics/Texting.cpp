#include "Texting.h"

#include "CameraUtilities.h"
#include "shaders.h"
#include "detail/UnitQuad.h"

#include <math/Transformations.h>

#include <renderer/Drawing.h>
#include <renderer/Uniforms.h>


namespace ad {
namespace graphics {


constexpr AttributeDescriptionList gGlyphInstanceDescription{
    { 2, 2, offsetof(Texting::Instance, position_w),    MappedGL<GLfloat>::enumerator},
    { {3, Attribute::Access::Integer}, 1, offsetof(Texting::Instance, offsetInTexture_p), MappedGL<GLint>::enumerator},
    { 4, 2, offsetof(Texting::Instance, boundingBox_p), MappedGL<GLfloat>::enumerator},
    { 5, 2, offsetof(Texting::Instance, bearing_p),     MappedGL<GLfloat>::enumerator},
};


constexpr GLint gTextureWidthCount = 256;

Texting::Texting(filesystem::path aFontPath,
                 GLfloat aGlyphWorldHeight, 
                 GLfloat aScreenWorldHeight,
                 std::shared_ptr<AppInterface> aAppInterface) :
    mQuadVbo{
        loadUnattachedVertexBuffer<detail::VertexUnitQuad>(
            detail::make_RectangleVertices({ {0.f, -1.f}, {1.f, 1.f} }))
    },
    mVaoPool{
        [this]()
        {
            VertexArrayObject vao;
            attachVertexBuffer<detail::VertexUnitQuad>(mQuadVbo, vao, detail::gVertexScreenDescription);
            VertexBufferObject instanceBuffer = initVertexBuffer<Instance>(vao, gGlyphInstanceDescription, 1);
            return PerTextureVao{
                std::move(vao),
                std::move(instanceBuffer)
            };
        }
    },
    mGpuProgram{makeLinkedProgram({
        {GL_VERTEX_SHADER,   texting::gGlyphVertexShader},
        {GL_FRAGMENT_SHADER, texting::gGlyphFragmentShader},
    })},
    mFontFace{mFreetype.load(aFontPath)},
    mPixelToWorld{decltype(mPixelToWorld)::Zero()}
{
    setCameraTransformation(math::AffineMatrix<3, GLfloat>::Identity());
    setProjectionTransformation(
        math::trans2d::orthographicProjection<GLfloat>(
            getViewRectangle(aAppInterface->getFramebufferSize(), aScreenWorldHeight)));

    GLfloat glyphPixelHeight =
        aGlyphWorldHeight * aAppInterface->getFramebufferSize().height() / aScreenWorldHeight;
    GLfloat pixelToWorld = aGlyphWorldHeight / glyphPixelHeight;
    mPixelToWorld = {pixelToWorld, pixelToWorld};
    setUniform(mGpuProgram, "u_PixelToWorld", mPixelToWorld);

    setUniformInt(mGpuProgram, "u_FontAtlas", gTextureUnit);

    mGlyphCache = detail::DynamicGlyphCache{
        {(GLint)(gTextureWidthCount * glyphPixelHeight),
         (GLint)(glyphPixelHeight + 1)}
    };

    // Font setup
    mFontFace.inverseYAxis(true);
    mFontFace.setPixelHeight(glyphPixelHeight);
}


void Texting::loadGlyphs(arte::CharCode aFirst, arte::CharCode aLast)
{
    mGlyphCache.preloadGlyphs(mFontFace, aFirst, aLast);
}


void Texting::render() const
{
    glUseProgram(mGpuProgram);
    glActiveTexture(GL_TEXTURE0 + gTextureUnit);

    // glDrawElementsInstancedBaseInstance not supported on macOS,
    // so workaround using several instance buffers.
    for (const handy::Pooled<PerTextureVao> & perTexture : mPerTexture)
    {
        glBindVertexArray(perTexture->vao);
        bind(*perTexture->texture);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, gVertexCount, perTexture->instanceCount);
    }
}


void Texting::setCameraTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation)
{
    setUniform(mGpuProgram, "u_WorldToCamera", aTransformation); 
}


void Texting::setProjectionTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation)
{
    setUniform(mGpuProgram, "u_Projection", aTransformation); 
}


math::Rectangle<GLfloat> Texting::getStringBounds(const std::string & aString, math::Position<2, GLfloat> aPenOrigin_w)
{
    math::Vec<2, GLfloat> pixelToWorld = mPixelToWorld.as<math::Vec>();
    math::Rectangle<GLfloat> boundingBox{aPenOrigin_w, {0.f, 0.f}};

    // Get rid of the first character bearing 
    // (i.e. difference between initial pen position, and a corner of the glyph bounding box).
    if(!aString.empty())
    {
        forEachGlyph({aString.begin(), aString.begin()+1}, aPenOrigin_w, mGlyphCache, mFontFace, mPixelToWorld,
            [&boundingBox, pixelToWorld](const detail::RenderedGlyph & rendered, math::Position<2, GLfloat> penPosition_w)
            {
                boundingBox.origin() += rendered.bearing.cwMul(pixelToWorld);
            });
    }

    forEachGlyph(aString, aPenOrigin_w, mGlyphCache, mFontFace, mPixelToWorld,
        [&boundingBox, pixelToWorld](const detail::RenderedGlyph & rendered, math::Position<2, GLfloat> penPosition_w)
        {
            // Those computations should work for both horizontal and vertical layouts.
            // Top-left corner of the glyph bounding box.
            boundingBox.extendTo(penPosition_w 
                                    + rendered.bearing.cwMul(pixelToWorld)); 
            // Bottom-right corner of the glyph bounding box.
            boundingBox.extendTo(penPosition_w 
                                    + rendered.bearing.cwMul(pixelToWorld)
                                    + math::Vec<2, GLfloat>{
                                        +rendered.controlBoxSize.width(),
                                        -rendered.controlBoxSize.height()}.cwMul(pixelToWorld));
        });

    return boundingBox;
}


} // namespace graphics
} // namespace ad
