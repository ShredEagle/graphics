#pragma once


#include <arte/Freetype.h>

#include <math/Vector.h>

#include <renderer/Texture.h>

#include <glad/glad.h>

#include <unordered_map>


namespace ad {
namespace graphics {
namespace detail {


/// Unidimensionnal array of rasters
struct TextureRibon
{
    /// \param aMargins The empty margin on each side of the glyph. 
    /// This is the margin that will be left empty on the left and below each glyph when copying
    /// their bitmap to the texture.
    /// In particular, this makes no guarantee about the margin on top, which
    /// is implicitly texture_height - glyph_height - margin_y.
    TextureRibon(Texture aTexture, GLint aWidth, math::Vec<2, GLint> aMargins) :
        texture{std::move(aTexture)},
        width{aWidth},
        margins{std::move(aMargins)}
    {}

    Texture texture;
    GLint width{0};
    math::Vec<2, GLint> margins;
    GLint nextXOffset = margins.x(); // Add the margin before the first glyph

    static constexpr math::Vec<2, GLint> gRecommendedMargins{2, 1};

    GLint isFitting(GLint aCandidateWidth)
    { return aCandidateWidth <= (width - nextXOffset); }

    GLint write(const std::byte * aData, InputImageParameters aInputParameters);
};


// Note: Linear offers smoother translations, at the cost of sharpness.
// Note: Nearest currently has a drawback that all letters of a string do not necessarily advance a pixel together.
inline TextureRibon make_TextureRibon(math::Size<2, GLint> aDimensions, GLenum aInternalFormat, math::Vec<2, GLint> aMargins, GLenum aTextureFiltering)
{
    TextureRibon ribon{Texture{GL_TEXTURE_RECTANGLE}, aDimensions.width(), aMargins};
    allocateStorage(ribon.texture, aInternalFormat, aDimensions.width(), aDimensions.height());
    // Note: Only the first (red) value will be used for a GL_R8 texture, but the API requires a 4-channel color.
    clear(ribon.texture, {math::hdr::gBlack, 0.f});

    bind(ribon.texture);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, aTextureFiltering);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, aTextureFiltering);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);

    return ribon;
}


inline GLint TextureRibon::write(const std::byte * aData, InputImageParameters aInputParameters)
{
    writeTo(texture, aData, aInputParameters, {nextXOffset, margins.y()});
    GLint thisOffset = nextXOffset;
    nextXOffset += aInputParameters.resolution.width() + margins.x();
    return thisOffset;
}


struct RenderedGlyph
{
    // TODO Storing a naked texture pointer is not ideal
    Texture * texture;
    GLint offsetInTexture; // The texture is a "1D" strip, only horizontal position.
    math::Size<2, GLfloat> controlBoxSize;
    math::Vec<2, GLfloat> bearing;
    math::Vec<2, GLfloat> penAdvance;
    unsigned int freetypeIndex; // Notably usefull for kerning queries.
};


using GlyphMap = std::unordered_map<arte::CharCode, RenderedGlyph>;


inline GLfloat fixedToFloat(FT_Pos aPos, int aFixedDecimals = 6)
{
    return (GLfloat)aPos / (1 << aFixedDecimals);
}


Texture makeTightGlyphAtlas(const arte::FontFace & aFontFace,
                            arte::CharCode aFirst, arte::CharCode aLast,
                            GlyphMap & aGlyphMap,
                            math::Vec<2, GLint> aMargins = TextureRibon::gRecommendedMargins);


struct StaticGlyphCache
{
    Texture atlas{0};
    GlyphMap glyphMap;
    arte::CharCode placeholder = 0x3F; // '?'

    // The empty cache
    StaticGlyphCache() = default;

    StaticGlyphCache(const arte::FontFace & aFontFace,
                     arte::CharCode aFirst, arte::CharCode aLast,
                     math::Vec<2, GLint> aDimensionExtension = {1, 0});

    RenderedGlyph at(arte::CharCode aCharCode) const;
};


struct DynamicGlyphCache
{
    // Since we are storing texture pointer, growing the atlas should not re-allocate!
    std::list<TextureRibon> atlases;
    GlyphMap glyphMap;
    GLenum textureFiltering = GL_LINEAR;
    math::Size<2, GLint> ribonDimension = {0, 0};
    math::Vec<2, GLint> margins = {0, 0};
    arte::CharCode placeholder = 0x3F; // '?'

    DynamicGlyphCache() = default;

    // The empty cache
    DynamicGlyphCache(math::Size<2, GLint> aRibonDimension_p, math::Vec<2, GLint> aMargins, GLenum aTextureFiltering);

    void growAtlas()
    {
        atlases.push_back(make_TextureRibon(ribonDimension, GL_RGB8, margins, textureFiltering));
    }

    RenderedGlyph at(arte::CharCode aCharCode, const arte::FontFace & aFontFace);

    void preloadGlyphs(const arte::FontFace & aFontFace, arte::CharCode aFirst, arte::CharCode aLast)
    {
        for(; aFirst != aLast; ++aFirst)
        {
            at(aFirst, aFontFace);
        }
    }
};


// TODO aPixelToLocal should be removed, when the Texting rendering does all "local layout" in pixel coordinates
void forEachGlyph(const std::string & aString,
                  math::Position<2, GLfloat> aPenOrigin_w,
                  DynamicGlyphCache & aGlyphCache, 
                  arte::FontFace & aFontFace,
                  math::Size<2, GLfloat> aPixelToLocal,
                  std::function<void(RenderedGlyph, math::Position<2, GLfloat>)> aGlyphCallback);


} // namespace detail
} // namespace graphics
} // namespace ad
