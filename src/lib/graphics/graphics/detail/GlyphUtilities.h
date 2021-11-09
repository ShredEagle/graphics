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

template <class T_atlas>
struct AtlasRepository
{
    std::vector<T_atlas> atlases;
};


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
                            GlyphMap & aGlyphCache,
                            math::Vec<2, GLint> aDimensionExtension = {1, 0});


} // namespace detail
} // namespace graphics
} // namespace ad
