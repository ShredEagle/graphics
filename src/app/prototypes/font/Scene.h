#pragma once


#include "Freetype.h"

#include "shaders.h"

#include <graphics/AppInterface.h>
#include <graphics/CameraUtilities.h>
#include <graphics/Timer.h>
#include <graphics/shaders.h>

#include <graphics/detail/UnitQuad.h>

#include <renderer/Drawing.h>
#include <renderer/Texture.h>
#include <renderer/Uniforms.h>

#include <math/Homogeneous.h>
#include <math/Vector.h>

#include <GLFW/glfw3.h>


using namespace ad::graphics;

namespace ad {
namespace font {


using Vec2 = math::Vec<2, GLfloat>;
using Position2 = math::Position<2, GLfloat>;
using Size2 = math::Size<2, GLfloat>;

constexpr GLint gTextureUnit = 2;
constexpr FT_ULong gStartingCharcode = 0x61; // 'a'


/// \brief A glyph info in the cache
struct Glyph
{
    GLint offsetInTexture; // The texture is a "1D" strip, only horizontal position.
    math::Size<2, GLfloat> boundingBox;
    Vec2 bearing;
    Vec2 penAdvance;
};


struct GlyphInstance
{
    GlyphInstance(Position2 aPosition_w, const Glyph & aGlyph) :
        position_w{std::move(aPosition_w)},
        offsetInTexture{aGlyph.offsetInTexture},
        boundingBox{aGlyph.boundingBox},
        bearing{aGlyph.bearing}
    {}

    Position2 position_w;
    GLint offsetInTexture;
    math::Size<2, GLfloat> boundingBox;
    math::Vec<2, GLfloat> bearing;
};


constexpr AttributeDescriptionList gGlyphInstanceDescription{
    { 2, 2, offsetof(GlyphInstance, position_w),  MappedGL<GLfloat>::enumerator},
    { {3, Attribute::Access::Integer}, 1, offsetof(GlyphInstance, offsetInTexture), MappedGL<GLint>::enumerator},
    { 4, 2, offsetof(GlyphInstance, boundingBox), MappedGL<GLfloat>::enumerator},
    { 5, 2, offsetof(GlyphInstance, bearing),     MappedGL<GLfloat>::enumerator},
};


struct Scene
{
    Scene(const filesystem::path & aFontPath,
          int aGlyphPixelHeight,
          GLfloat aGlyphWorldHeight, 
          GLfloat aScreenWorldHeight,
          graphics::AppInterface & aAppInterface);

    void step(const Timer & aTimer);
    void render();

    void onKey(int key, int scancode, int action, int mods);

    Size2 mPixelToWorld;
    std::vector<Glyph> mGlyphCache;
    graphics::Texture mFontAtlas;
    graphics::Program mPassthrough;
    graphics::Program mFontProgram;
    graphics::VertexSpecification mScreenQuadVertex{graphics::detail::make_UnitQuad()};
    graphics::VertexSpecification mGlyphQuadVertex{
        graphics::detail::make_Rectangle({ {0.f, -1.f}, {1.f, 1.f} })};
    bool mShowAtlas{false};
    GLsizei mGlyphCount{0};
};


// Hardcoded for 26.6
GLfloat toFloat(FT_Pos aPos)
{
    return (GLfloat)aPos / (1 << 6);
}


inline Scene::Scene(const filesystem::path & aFontPath,
                    int aGlyphPixelHeight,
                    GLfloat aGlyphWorldHeight, 
                    GLfloat aScreenWorldHeight,
                    graphics::AppInterface & aAppInterface) :
    mPixelToWorld{aGlyphWorldHeight / aGlyphPixelHeight, aGlyphWorldHeight / aGlyphPixelHeight},
    mFontAtlas{GL_TEXTURE_RECTANGLE},
    mPassthrough{ makeLinkedProgram({
        {GL_VERTEX_SHADER, graphics::gPassthroughVertexShader},
        {GL_FRAGMENT_SHADER, gFontFragmentShader}
    })},
    mFontProgram{ makeLinkedProgram({
        {GL_VERTEX_SHADER,   gFontVertexShader},
        {GL_FRAGMENT_SHADER, gFontFragmentShader}
    })}
{
    // Keyboard callback
    using namespace std::placeholders;
    aAppInterface.registerKeyCallback(std::bind(&Scene::onKey, this, _1, _2, _3, _4));

    // Glyph instance buffer
    mGlyphQuadVertex.mVertexBuffers.push_back(
        initVertexBuffer<GlyphInstance>(mGlyphQuadVertex.mVertexArray, gGlyphInstanceDescription, 1));

    // Screen program
    setUniformInt(mPassthrough, "inputTexture", gTextureUnit);

    // Font program
    setUniform(mFontProgram, "u_PixelToWorld", mPixelToWorld);
    setUniform(mFontProgram, "u_WorldToCamera", math::AffineMatrix<4, GLfloat>::Identity());
    setUniform(mFontProgram, "u_Projection", 
        math::trans3d::orthographicProjection<GLfloat>(
            getViewVolume(aAppInterface.getFramebufferSize(), aScreenWorldHeight, 1, -1)));

    
    // Font atlas generation
    auto startingCharcode = gStartingCharcode;
    constexpr int glyphPerLine = 20;
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
            GlyphSlot glyph = face.getGlyph(charcode);
            GlyphBitmap bitmap = glyph.render();
            InputImageParameters inputParams{
                {bitmap.width(), bitmap.rows()},
                GL_RED,
                GL_UNSIGNED_BYTE,
                1
            };

            if ( (textureX + bitmap.width()) > textureWidth )
            {
                break;
            }
            writeTo(mFontAtlas, bitmap.data(), inputParams, {textureX, 0});

            mGlyphCache.push_back({
                textureX,
                {toFloat(glyph.metric().width), toFloat(glyph.metric().height)},
                {toFloat(glyph.metric().horiBearingX), toFloat(glyph.metric().horiBearingY)},
                {toFloat(glyph.metric().horiAdvance), 0.f /* hardcoded horizontal layout */},
            });

            textureX += bitmap.width();
        }
    }

    setUniform(mPassthrough, "u_UVScaling", Size2{(GLfloat)textureX, (GLfloat)aGlyphPixelHeight});
}


void Scene::onKey(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
    {
        mShowAtlas = !mShowAtlas;
    }
}


inline void Scene::step(const Timer & aTimer)
{
    //const std::string = "Salut, monde.";
    const std::string message{"salutmonde"};
    Position2 penPosition{-20.f, 0.f};

    std::vector<GlyphInstance> instances;
    for (char c : message)
    {
        std::size_t cacheIndex = (int)c - gStartingCharcode;
        instances.emplace_back(penPosition, mGlyphCache[cacheIndex]);
        penPosition += mGlyphCache[cacheIndex].penAdvance.cwMul(mPixelToWorld.as<math::Vec>());
    }

    respecifyBuffer<GlyphInstance>(mGlyphQuadVertex.mVertexBuffers.back(), instances);
    mGlyphCount = instances.size();
}


inline void Scene::render()
{
    glActiveTexture(GL_TEXTURE0 + gTextureUnit);
    bind(mFontAtlas);

    if (mShowAtlas)
    {
        activate(mScreenQuadVertex, mPassthrough);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    else
    {
        activate(mGlyphQuadVertex, mFontProgram);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, mGlyphCount);
    }
}


} // namespace font
} // namespace ad