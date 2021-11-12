#include "GlyphUtilities.h"

#include "Logging.h"


namespace ad {
namespace graphics {
namespace detail {


StaticGlyphCache::StaticGlyphCache(const arte::FontFace & aFontFace,
                                   arte::CharCode aFirst, arte::CharCode aLast,
                                   math::Vec<2, GLint> aDimensionExtension)
{
    atlas = makeTightGlyphAtlas(aFontFace, aFirst, aLast, glyphMap, aDimensionExtension);
}


RenderedGlyph StaticGlyphCache::at(arte::CharCode aCharCode) const
{
    if(auto found = glyphMap.find(aCharCode); found != glyphMap.end())
    {
        return found->second;
    }
    else
    {
        return glyphMap.at(placeholder);
    }
}


RenderedGlyph DynamicGlyphCache::at(arte::CharCode aCharCode, const arte::FontFace & aFontFace)
{
    if(auto found = glyphMap.find(aCharCode); found != glyphMap.end())
    {
        return found->second;
    }
    else
    {
        LOG(graphics, trace)("Glyph for charcode {} not found in cache, rendering.", aCharCode);
        if (aFontFace.hasGlyph(aCharCode))
        {
            arte::GlyphSlot slot = aFontFace.getGlyphSlot(aCharCode);
            arte::GlyphBitmap bitmap = slot.render();

            if (!atlases.back().isFitting(bitmap.width()))
            {
                LOG(graphics, info)("Growing dynamic atlas with a new texture.", aCharCode);
                growAtlas();
            }
            assert(atlases.back().isFitting(bitmap.width()));

            InputImageParameters inputParams{
                {bitmap.width(), bitmap.rows()},
                GL_RED,
                GL_UNSIGNED_BYTE,
                1
            };

            RenderedGlyph rendered{
                &atlases.back().texture,
                atlases.back().write(bitmap.data(), inputParams),
                {fixedToFloat(slot.metric().width), fixedToFloat(slot.metric().height)},
                {fixedToFloat(slot.metric().horiBearingX), fixedToFloat(slot.metric().horiBearingY)},
                {fixedToFloat(slot.metric().horiAdvance), 0.f /* hardcoded horizontal layout */},
                slot.index()
            };
            return glyphMap.insert({aCharCode, rendered}).first->second;
        }
        else
        {
            LOG(graphics, warn)("No glyph for character code: {}", aCharCode);
        }
    }
    assert(aCharCode != placeholder); // otherwise infinity open its time consuming arms
    return at(placeholder, aFontFace);
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
                    nullptr,
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
        rendered.texture = &ribon.texture;
        rendered.offsetInTexture = ribon.write(bitmap.data(), inputParams);
        aGlyphMap.insert({charcode, rendered});
    }

    return std::move(ribon.texture);
}


} // namespace detail
} // namespace graphics
} // namespace ad
