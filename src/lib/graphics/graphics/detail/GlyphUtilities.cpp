#include "GlyphUtilities.h"


namespace ad {
namespace graphics {
namespace detail {


StaticGlyphCache::StaticGlyphCache(const arte::FontFace & aFontFace,
                                   arte::CharCode aFirst, arte::CharCode aLast,
                                   math::Vec<2, GLint> aDimensionExtension)
{
    atlas = makeTightGlyphAtlas(aFontFace, aFirst, aLast, glyphMap, aDimensionExtension);
}

Texture makeTightGlyphAtlas(const arte::FontFace & aFontFace,
                            arte::CharCode aFirst, arte::CharCode aLast,
                            GlyphMap & aGlyphMap,
                            math::Vec<2, GLint> aDimensionExtension)
{
    std::vector<std::tuple<arte::CharCode, arte::Glyph, RenderedGlyph>> glyphs;

    //
    // Compute the atlas dimension
    //
    math::Size<2, GLint> atlasDimensions{0, 0};
    for(; aFirst != aLast; ++aFirst)
    {
        if (aFontFace.hasGlyph(aFirst))
        {
            arte::GlyphSlot slot = aFontFace.getGlyphSlot(aFirst);
            // Note: The glyph metrics available in FT_GlyphSlot are not available in FT_Glyph
            // so we store them now...
            // see: https://lists.gnu.org/archive/html/freetype/2010-09/msg00036.html
            glyphs.push_back({
                aFirst,
                slot.getGlyph(),
                RenderedGlyph{
                    0,
                    0,
                    {fixedToFloat(slot.metric().width), fixedToFloat(slot.metric().height)},
                    {fixedToFloat(slot.metric().horiBearingX), fixedToFloat(slot.metric().horiBearingY)},
                    {fixedToFloat(slot.metric().horiAdvance), 0.f /* hardcoded horizontal layout */},
                    slot.index()
                }
            });
            math::Size<2, int> glyphSize = std::get<1>(glyphs.back()).getBoundingDimension();
            atlasDimensions.width() += glyphSize.width();
            atlasDimensions.height() = std::max(atlasDimensions.height(), glyphSize.height());

            atlasDimensions.width() += aDimensionExtension.x();
        }
    }
    if (atlasDimensions.width() != 0) // Check if it is not an empty set of glyphs
    {
        // Remove the extra extension at the end.
        atlasDimensions.width() -= aDimensionExtension.x();
    }
    atlasDimensions.height() += aDimensionExtension.y();

    //
    // Fill in the atlas
    //
    TextureRibon ribon = make_TextureRibon(atlasDimensions, GL_R8);
    ribon.margin = aDimensionExtension.x();
    for (auto & [charcode, glyph, rendered] : glyphs)
    {
        arte::GlyphBitmap bitmap = glyph.render();

        assert(ribon.isFitting(bitmap.width()));

        InputImageParameters inputParams{
            {bitmap.width(), bitmap.rows()},
            GL_RED,
            GL_UNSIGNED_BYTE,
            1
        };
        rendered.glTexture = ribon.texture;
        rendered.offsetInTexture = ribon.write(bitmap.data(), inputParams);
        aGlyphMap.insert({charcode, rendered});
    }

    return std::move(ribon.texture);
}


} // namespace detail
} // namespace graphics
} // namespace ad
