#include "GlyphUtilities.h"

#include "Logging.h"

#include <utf8.h> // utfcpp lib


namespace ad {
namespace graphics {
namespace detail {


StaticGlyphCache::StaticGlyphCache(const arte::FontFace & aFontFace,
                                   arte::CharCode aFirst, arte::CharCode aLast,
                                   math::Vec<2, GLint> aMargins)
{
    atlas = makeTightGlyphAtlas(aFontFace, aFirst, aLast, glyphMap, aMargins);
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

            auto & margins = atlases.back().margins;

            RenderedGlyph rendered{
                &atlases.back().texture,
                atlases.back().write(bitmap.data(), inputParams) - (margins.x() / 2), // go back half the horizontal margin (not full margin to avoid bleeding)
                //{fixedToFloat(slot.metric().width), fixedToFloat(slot.metric().height)},
                // Note: We observe noticeable "edge trimming" when using the exact glyph bounding box.
                // This is because a fragment is generated only if the primitive hits the center of the pixel.
                // Adding margin on each dimension allows to make sure the top and right border pixels are not discarded, when rendering at matching resolution.
                // Important: add half horizontal margin on each side to avoid bleeding, and whole vertical margin on top and bottom (no vertical bleeding in a ribbon).
                {fixedToFloat(slot.metric().width) + margins.x(), fixedToFloat(slot.metric().height) + 2 * margins.y()}, 
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


void forEachGlyph(const std::string & aString,
                  math::Position<2, GLfloat> aPenOrigin_w,
                  DynamicGlyphCache & aGlyphCache, 
                  arte::FontFace & aFontFace,
                  math::Size<2, GLfloat> aPixelToLocal,
                  std::function<void(RenderedGlyph, math::Position<2, GLfloat>)> aGlyphCallback)
{
    unsigned int previousIndex = 0;
    for (std::string::const_iterator it = aString.begin();
         it != aString.end();
         /* in body */)
    {
        // Decode utf8 encoded string to individual Unicode code points
        arte::CharCode codePoint = utf8::next(it, aString.end());
        detail::RenderedGlyph rendered = aGlyphCache.at(codePoint, aFontFace);
        
        // Kerning
        if (previousIndex != 0)
        {
            Vec2<GLfloat> kerning = aFontFace.kern(previousIndex, rendered.freetypeIndex);
            aPenOrigin_w += kerning.cwMul(aPixelToLocal.as<math::Vec>());
        }
        previousIndex = rendered.freetypeIndex;

        aGlyphCallback(rendered, aPenOrigin_w);
        aPenOrigin_w += rendered.penAdvance.cwMul(aPixelToLocal.as<math::Vec>());
    }
}


Texture makeTightGlyphAtlas(const arte::FontFace & aFontFace,
                            arte::CharCode aFirst, arte::CharCode aLast,
                            GlyphMap & aGlyphMap,
                            math::Vec<2, GLint> aMargins)
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
                    // See DynamicGlyphCache::at() for the ratrionale behind the addition
                    {fixedToFloat(slot.metric().width) + aMargins.x(), fixedToFloat(slot.metric().height) + 2 * aMargins.y()}, 
                    {fixedToFloat(slot.metric().horiBearingX), fixedToFloat(slot.metric().horiBearingY)},
                    {fixedToFloat(slot.metric().horiAdvance), 0.f /* hardcoded horizontal layout */},
                    slot.index()
                }
            });
            math::Size<2, int> glyphSize = std::get<1>(glyphs.back()).getBoundingDimension();
            atlasDimensions.width() += glyphSize.width();
            atlasDimensions.height() = std::max(atlasDimensions.height(), glyphSize.height());

            atlasDimensions.width() += aMargins.x();
        }
    }
    // The margin is present in the ribon before the first glyph, no need to remove the initial x extension
    atlasDimensions.height() += 2 * aMargins.y();

    //
    // Fill in the atlas
    //
    TextureRibon ribon = make_TextureRibon(atlasDimensions, GL_R8, aMargins);
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
        rendered.offsetInTexture = ribon.write(bitmap.data(), inputParams) - (aMargins.x() / 2);
        aGlyphMap.insert({charcode, rendered});
    }

    return std::move(ribon.texture);
}


} // namespace detail
} // namespace graphics
} // namespace ad
