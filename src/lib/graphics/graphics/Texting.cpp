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
    setCameraTransformation(math::AffineMatrix<4, GLfloat>::Identity());
    setProjectionTransformation(
        math::trans3d::orthographicProjection<GLfloat>(
            getViewVolume(aAppInterface->getFramebufferSize(), aScreenWorldHeight, 1, -1)));

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
    for (const Pooled<PerTextureVao> & perTexture : mPerTexture)
    {
        glBindVertexArray(perTexture->vao);
        bind(*perTexture->texture);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, gVertexCount, perTexture->instanceCount);
    }
}


void Texting::setCameraTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation)
{
    setUniform(mGpuProgram, "u_WorldToCamera", aTransformation); 
}


void Texting::setProjectionTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation)
{
    setUniform(mGpuProgram, "u_Projection", aTransformation); 
}


} // namespace graphics
} // namespace ad
