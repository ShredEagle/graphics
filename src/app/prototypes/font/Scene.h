#pragma once


#include "Freetype.h"

#include "shaders.h"

#include <graphics/Timer.h>
#include <graphics/shaders.h>

#include <graphics/detail/UnitQuad.h>

#include <renderer/Drawing.h>
#include <renderer/Texture.h>
#include <renderer/Uniforms.h>

#include <math/Vector.h>

using namespace ad::graphics;

namespace ad {
namespace font {


using Vec2 = math::Vec<2, GLfloat>;

constexpr GLint gTextureUnit = 2;

//constexpr AttributeDescriptionList gBezierInstanceDescription{
//    {2, 2, offsetof(CubicBezier, p) + 0 * sizeof(Vec2), MappedGL<GLfloat>::enumerator},
//    {3, 2, offsetof(CubicBezier, p) + 1 * sizeof(Vec2), MappedGL<GLfloat>::enumerator},
//    {4, 2, offsetof(CubicBezier, p) + 2 * sizeof(Vec2), MappedGL<GLfloat>::enumerator},
//    {5, 2, offsetof(CubicBezier, p) + 3 * sizeof(Vec2), MappedGL<GLfloat>::enumerator},
//};


struct Scene
{
    Scene(const filesystem::path & aFontPath, int aGlyphPixelHeight);

    void step(const Timer & aTimer);
    void render();

    graphics::Texture mFontAtlas;
    graphics::VertexSpecification mQuadVertex{graphics::detail::make_UnitQuad()};
    graphics::Program mPassthrough;
};


inline Scene::Scene(const filesystem::path & aFontPath, int aGlyphPixelHeight) :
    mFontAtlas{GL_TEXTURE_2D},
    mPassthrough{ makeLinkedProgram({
        {GL_VERTEX_SHADER, graphics::gPassthroughVertexShader},
        {GL_FRAGMENT_SHADER, gFontFragmentShader}
    })}
{
    // Program
    setUniformInt(mPassthrough, "inputTexture", gTextureUnit);

    // Font atlas generation
    constexpr int glyphPerLine = 20;
    FT_ULong startingCharcode = 0x61; // 'a'
    GLsizei textureWidth = glyphPerLine * aGlyphPixelHeight;
    allocateStorage(mFontAtlas, GL_R8, textureWidth, aGlyphPixelHeight);

    Freetype freetype;
    FontFace face = freetype.load(aFontPath);
    face.inverseYAxis(true);
    face.setPixelHeight(aGlyphPixelHeight);
    GLint textureX = 0;
    while(true)
    {
        auto charcode = startingCharcode++;
        if (face.hasGlyph(charcode))
        {
            GlyphBitmap glyph = face.getGlyph(charcode);
            InputImageParameters inputParams{
                {glyph.width(), glyph.rows()},
                GL_RED,
                GL_UNSIGNED_BYTE,
                1
            };

            if ( (textureX + glyph.width()) > textureWidth )
            {
                break;
            }
            writeTo(mFontAtlas, glyph.data(), inputParams, {textureX, 0});
            textureX += glyph.width();
        }
    }
}


inline void Scene::step(const Timer & aTimer)
{
}


inline void Scene::render()
{
    activate(mQuadVertex, mPassthrough);

    glActiveTexture(GL_TEXTURE0 + gTextureUnit);
    bind(mFontAtlas);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


} // namespace font
} // namespace ad
