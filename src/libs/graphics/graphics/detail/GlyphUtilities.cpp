#include "GlyphUtilities.h"

#include "Logging.h"

#include <renderer/Query.h>

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


DynamicGlyphCache::DynamicGlyphCache(math::Size<2, GLint> aRibonDimension_p,
                                     math::Vec<2, GLint> aMargins,
                                     GLenum aTextureFiltering) :
    textureFiltering{aTextureFiltering},
    ribonDimension{[&]() -> math::Size<2, GLint>
        {
            if(aRibonDimension_p.height() > getMaxTextureSize())
            {
                throw std::invalid_argument{"Texture ribon too tall."};
            }
            else if(aRibonDimension_p.width() > getMaxTextureSize())
            {
                ADLOG(gMainLogger, warn)("Requested ribon dimension ({}, {}) exceeds maximum texture dimension {}. Clamping the width.",
                    aRibonDimension_p.width(), aRibonDimension_p.height(), getMaxTextureSize());
                return {getMaxTextureSize(), aRibonDimension_p.height()};
            }
            return aRibonDimension_p;
        }()},
    margins{aMargins}
{
    growAtlas();
}


RenderedGlyph DynamicGlyphCache::at(arte::CharCode aCharCode, const arte::FontFace & aFontFace)
{
    if(auto found = glyphMap.find(aCharCode); found != glyphMap.end())
    {
        return found->second;
    }
    else
    {
        ADLOG(gMainLogger, trace)("Glyph for charcode {} not found in cache, rendering.", aCharCode);
        if (aFontFace.hasGlyph(aCharCode))
        {
            arte::GlyphSlot slot = aFontFace.getGlyphSlot(aCharCode);
            arte::GlyphBitmap bitmap = slot.render();

            if (!atlases.back().isFitting(bitmap.width()))
            {
                ADLOG(gMainLogger, info)("Growing dynamic atlas with a new texture.", aCharCode);
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
                atlases.back().write(bitmap.data(), inputParams),
                // Note: We observe noticeable "edge trimming" when using the exact glyph bounding box.
                // This is because a fragment is generated only if the primitive hits the center of the pixel.
                // Adding margin on each dimension allows to make sure the top and right border pixels are not discarded, when rendering at matching resolution.
                // Important: The margin is added to each side, so each glyph dimension is agumented by 2 horizontal and 2 vertical margins.
                {fixedToFloat(slot.metric().width) + 2 * margins.x(), fixedToFloat(slot.metric().height) + 2 * margins.y()}, 
                // The bearing goes **back** the left horizontal margin, and goes **up** the top vertical margin.
                {fixedToFloat(slot.metric().horiBearingX) - margins.x(), fixedToFloat(slot.metric().horiBearingY) + margins.y()},
                {fixedToFloat(slot.metric().horiAdvance), 0.f /* hardcoded horizontal layout */},
                slot.index()
            };
            return glyphMap.insert({aCharCode, rendered}).first->second;
        }
        else
        {
            ADLOG(gMainLogger, warn)("No glyph for character code: {}", aCharCode);
        }
    }
    assert(aCharCode != placeholder); // otherwise infinity open its time consuming arms
    return at(placeholder, aFontFace);
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
                    {fixedToFloat(slot.metric().width) + 2 * aMargins.x(), fixedToFloat(slot.metric().height) + 2 * aMargins.y()}, 
                    {fixedToFloat(slot.metric().horiBearingX) - aMargins.x(), fixedToFloat(slot.metric().horiBearingY) + aMargins.y()},
                    {fixedToFloat(slot.metric().horiAdvance), 0.f /* hardcoded horizontal layout */},
                    slot.index()
                }
            });
            math::Size<2, int> glyphSize = std::get<1>(glyphs.back()).getBoundingDimension();
            atlasDimensions.width() += glyphSize.width() + 2 * aMargins.x();
            atlasDimensions.height() = std::max(atlasDimensions.height(), glyphSize.height());
        }
    }
    atlasDimensions.height() += 2 * aMargins.y();

    //
    // Fill in the atlas
    //
    TextureRibon ribon = make_TextureRibon(atlasDimensions, GL_R8, aMargins, GL_LINEAR);
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
        rendered.texture = &ribon.texture; // Replace the nullptr with the actual the ribon texture.
        rendered.offsetInTexture = ribon.write(bitmap.data(), inputParams);
        aGlyphMap.insert({charcode, rendered});
    }

    return std::move(ribon.texture);
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


void forEachGlyph(const std::string & aString,
                  math::Position<2, GLfloat> aPenOrigin_p,
                  const StaticGlyphCache & aGlyphCache,
                  const arte::FontFace & aFontFace,
                  std::function<void(const RenderedGlyph &, math::Position<2, GLfloat>)> aGlyphCallback)
{
    unsigned int previousIndex = 0;
    for (std::string::const_iterator it = aString.begin();
         it != aString.end();
         /* in body */)
    {
        // Decode utf8 encoded string to individual Unicode code points
        arte::CharCode codePoint = utf8::next(it, aString.end());
        detail::RenderedGlyph rendered = aGlyphCache.at(codePoint);
        
        // Kerning
        if (previousIndex != 0)
        {
            aPenOrigin_p += aFontFace.kern(previousIndex, rendered.freetypeIndex);
        }
        previousIndex = rendered.freetypeIndex;

        aGlyphCallback(rendered, aPenOrigin_p);
        aPenOrigin_p += rendered.penAdvance;
    }
}


math::Size<2, GLfloat> getStringDimension(const std::string & aString,
                                          const StaticGlyphCache & aGlyphCache,
                                          const arte::FontFace & aFontFace)
{
    math::Size<2, GLfloat> result{static_cast<math::Size<2, GLfloat>>(aFontFace.getPixelSize())};
    forEachGlyph(aString, math::Position<2, GLfloat>{0.f, 0.f}, aGlyphCache, aFontFace, [&result](const auto & rendered, auto position)
    {
        result = (position + rendered.penAdvance).template as<math::Size>();
    });
    return result;
}


} // namespace detail
} // namespace graphics
} // namespace ad
