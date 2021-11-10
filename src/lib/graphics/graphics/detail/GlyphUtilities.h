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
    Texture texture;
    GLint width;
    GLint margin = 1;
    GLint nextXOffset = 0;

    GLint isFitting(GLint aCandidateWidth)
    { return aCandidateWidth <= (width - nextXOffset); }

    GLint write(const std::byte * aData, InputImageParameters aInputParameters);
};


inline TextureRibon make_TextureRibon(math::Size<2, GLint> aDimensions, GLenum aInternalFormat)
{
    TextureRibon ribon{Texture{GL_TEXTURE_RECTANGLE}, aDimensions.width()};
    allocateStorage(ribon.texture, aInternalFormat, aDimensions.width(), aDimensions.height());

    bind(ribon.texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return ribon;
}


inline GLint TextureRibon::write(const std::byte * aData, InputImageParameters aInputParameters)
{
    writeTo(texture, aData, aInputParameters, {nextXOffset, 0});
    GLint thisOffset = nextXOffset;
    nextXOffset += aInputParameters.resolution.width() + margin;
    return thisOffset;
}


struct RenderedGlyph
{
    GLuint glTexture;
    GLint offsetInTexture; // The texture is a "1D" strip, only horizontal position.
    math::Size<2, GLfloat> boundingBox;
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
                            math::Vec<2, GLint> aDimensionExtension = {1, 0});


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
    std::vector<TextureRibon> atlases;
    GlyphMap glyphMap;
    math::Size<2, GLint> ribonDimension = {0, 0};
    arte::CharCode placeholder = 0x3F; // '?'

    DynamicGlyphCache() = default;

    // The empty cache
    DynamicGlyphCache(math::Size<2, GLint> aRibonDimension_p) :
        ribonDimension{aRibonDimension_p}
    {
        growAtlas();
    }

    void growAtlas()
    {
        atlases.push_back(make_TextureRibon(ribonDimension, GL_RGB8));
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



} // namespace detail
} // namespace graphics
} // namespace ad
