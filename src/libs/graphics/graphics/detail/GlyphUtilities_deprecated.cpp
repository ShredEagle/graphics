#include "GlyphUtilities_deprecated.h"

#include "Logging.h"

#include <freetype/freetype.h>
#include <freetype/ftimage.h>
#include <renderer/SynchronousQueries.h>
#include <utf8.h> // utfcpp lib

namespace ad {
namespace graphics {
namespace detail {

StaticGlyphCache::StaticGlyphCache(const arte::FontFace & aFontFace,
                                   arte::CharCode aFirst,
                                   arte::CharCode aLast,
                                   math::Vec<2, GLint> aMargins)
{
    atlas = makeTightGlyphAtlas(aFontFace, aFirst, aLast, glyphMap, aMargins);
}

RenderedGlyph StaticGlyphCache::at(arte::CharCode aCharCode) const
{
    if (auto found = glyphMap.find(aCharCode); found != glyphMap.end())
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
    ribonDimension{[&]() -> math::Size<2, GLint> {
        if (aRibonDimension_p.height() > getMaxTextureSize())
        {
            throw std::invalid_argument{"Texture ribon too tall."};
        }
        else if (aRibonDimension_p.width() > getMaxTextureSize())
        {
            ADLOG(gMainLogger, warn)
            ("Requested ribon dimension ({}, {}) exceeds maximum texture "
             "dimension {}. Clamping the width.",
             aRibonDimension_p.width(), aRibonDimension_p.height(),
             getMaxTextureSize());
            return {getMaxTextureSize(), aRibonDimension_p.height()};
        }
        return aRibonDimension_p;
    }()},
    margins{aMargins}
{
    growAtlas();
}

RenderedGlyph DynamicGlyphCache::at(arte::CharCode aCharCode,
                                    const arte::FontFace & aFontFace)
{
    if (auto found = glyphMap.find(aCharCode); found != glyphMap.end())
    {
        return found->second;
    }
    else
    {
        ADLOG(gMainLogger, trace)
        ("Glyph for charcode {} not found in cache, rendering.", aCharCode);
        if (aFontFace.hasGlyph(aCharCode))
        {
            FT_GlyphSlot slot = aFontFace.loadChar(
                aCharCode,
                FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF));
            FT_Bitmap bitmap = aFontFace.renderGlyphSlot(FT_RENDER_MODE_NORMAL);

            if (!atlases.back().isFitting(bitmap.width))
            {
                ADLOG(gMainLogger, info)
                ("Growing dynamic atlas with a new texture.", aCharCode);
                growAtlas();
            }
            assert(atlases.back().isFitting(bitmap.width));

            InputImageParameters inputParams{
                {static_cast<int>(bitmap.width), static_cast<int>(bitmap.rows)},
                GL_RED,
                GL_UNSIGNED_BYTE,
                1};

            RenderedGlyph rendered{
                &atlases.back().texture,
                atlases.back().write(
                    reinterpret_cast<const std::byte *>(bitmap.buffer),
                    inputParams),
                // Note: We observe noticeable "edge trimming" when using the
                // exact glyph bounding box. This is because a fragment is
                // generated only if the primitive hits the center of the pixel.
                // Adding margin on each dimension allows to make sure the top
                // and right border pixels are not discarded, when rendering at
                // matching resolution. Important: The margin is added to each
                // side, so each glyph dimension is agumented by 2 horizontal
                // and 2 vertical margins.
                {fixedToFloat(slot->metrics.width),
                 fixedToFloat(slot->metrics.height)},
                {fixedToFloat(slot->metrics.horiBearingX),
                 fixedToFloat(slot->metrics.horiBearingY)},
                // The bearing goes **back** the left horizontal margin, and
                // goes **up** the top vertical margin.
                {fixedToFloat(slot->metrics.horiAdvance),
                 0.f /* hardcoded horizontal layout */},
                slot->glyph_index};
            return glyphMap.insert({aCharCode, rendered}).first->second;
        }
        else
        {
            ADLOG(gMainLogger, warn)
            ("No glyph for character code: {}", aCharCode);
        }
    }
    assert(aCharCode
           != placeholder); // otherwise infinity open its time consuming arms
    return at(placeholder, aFontFace);
}

Texture makeTightGlyphAtlas(const arte::FontFace & aFontFace,
                            arte::CharCode aFirst,
                            arte::CharCode aLast,
                            GlyphMap & aGlyphMap,
                            math::Vec<2, GLint> aMargins)
{
    std::vector<arte::CharCode> glyphs;

    //
    // Compute the atlas dimension
    //
    math::Size<2, GLint> atlasDimensions{0, 0};
    for (; aFirst != aLast; ++aFirst)
    {
        if (aFontFace.hasGlyph(aFirst))
        {
            FT_GlyphSlot slot = aFontFace.loadChar(
                aFirst, FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF));
            // Note: The glyph metrics available in FT_GlyphSlot are not
            // available in FT_Glyph so we store them now... see:
            // https://lists.gnu.org/archive/html/freetype/2010-09/msg00036.html
            glyphs.push_back(aFirst);
            math::Size<2, int> glyphSize{
                static_cast<int>(slot->bitmap.width) + 2 * aMargins.x(),
                static_cast<int>(slot->bitmap.rows) + 2 * aMargins.y()};
            atlasDimensions.width() += glyphSize.width();
            atlasDimensions.height() =
                std::max(atlasDimensions.height(), glyphSize.height());
        }
    }

    //
    // Fill in the atlas
    //
    TextureRibon ribon =
        make_TextureRibon(atlasDimensions, GL_R8, aMargins, GL_LINEAR);
    for (auto & charcode : glyphs)
    {
        FT_GlyphSlot slot = aFontFace.loadChar(
            charcode, FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF));
        FT_Bitmap bitmap = slot->bitmap;

        assert(ribon.isFitting(bitmap.width));

        InputImageParameters inputParams{
            {static_cast<int>(bitmap.width), static_cast<int>(bitmap.rows)},
            GL_RED,
            GL_UNSIGNED_BYTE,
            1};
        RenderedGlyph rendered{&ribon.texture,
                               ribon.write(
            reinterpret_cast<const std::byte *>(bitmap.buffer), inputParams),
                               // See DynamicGlyphCache::at() for the ratrionale
                               // behind the addition
                               {static_cast<float>(slot->bitmap.width) + 2 * aMargins.x(),
                                static_cast<float>(slot->bitmap.rows) + 2 * aMargins.y()},
                               {static_cast<float>(slot->bitmap_left) - 2 * aMargins.x(),
                                static_cast<float>(slot->bitmap.rows - slot->bitmap_top) + aMargins.y()},
                               {fixedToFloat(slot->metrics.horiAdvance),
                                0.f /* hardcoded horizontal layout */},
                               slot->glyph_index};
        rendered.texture = &ribon.texture; // Replace the nullptr with the
                                           // actual the ribon texture.
        aGlyphMap.insert({charcode, rendered});
    }

    return std::move(ribon.texture);
}

void forEachGlyph(const std::string & aString,
                  math::Position<2, GLfloat> aPenOrigin_w,
                  DynamicGlyphCache & aGlyphCache,
                  arte::FontFace & aFontFace,
                  math::Size<2, GLfloat> aPixelToLocal,
                  std::function<void(RenderedGlyph, math::Position<2, GLfloat>)>
                      aGlyphCallback)
{
    unsigned int previousIndex = 0;
    for (std::string::const_iterator it = aString.begin(); it != aString.end();
         /* in body */)
    {
        // Decode utf8 encoded string to individual Unicode code points
        arte::CharCode codePoint = utf8::next(it, aString.end());
        detail::RenderedGlyph rendered = aGlyphCache.at(codePoint, aFontFace);

        // Kerning
        if (previousIndex != 0)
        {
            Vec2<GLfloat> kerning =
                aFontFace.kern(previousIndex, rendered.freetypeIndex);
            aPenOrigin_w += kerning.cwMul(aPixelToLocal.as<math::Vec>());
        }
        previousIndex = rendered.freetypeIndex;

        aGlyphCallback(rendered, aPenOrigin_w);
        aPenOrigin_w +=
            rendered.penAdvance.cwMul(aPixelToLocal.as<math::Vec>());
    }
}

void forEachGlyph(
    const std::string & aString,
    math::Position<2, GLfloat> aPenOrigin_p,
    const StaticGlyphCache & aGlyphCache,
    const arte::FontFace & aFontFace,
    std::function<void(const RenderedGlyph &, math::Position<2, GLfloat>)>
        aGlyphCallback)
{
    unsigned int previousIndex = 0;
    for (std::string::const_iterator it = aString.begin(); it != aString.end();
         /* in body */)
    {
        // Decode utf8 encoded string to individual Unicode code points
        arte::CharCode codePoint = utf8::next(it, aString.end());
        detail::RenderedGlyph rendered = aGlyphCache.at(codePoint);

        // Kerning
        if (previousIndex != 0)
        {
            aPenOrigin_p +=
                aFontFace.kern(previousIndex, rendered.freetypeIndex);
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
    math::Size<2, GLfloat> result{
        static_cast<math::Size<2, GLfloat>>(aFontFace.getPixelSize())};
    forEachGlyph(
        aString, math::Position<2, GLfloat>{0.f, 0.f}, aGlyphCache, aFontFace,
        [&result](const auto & rendered, auto position) {
            result = (position + rendered.penAdvance).template as<math::Size>();
        });
    return result;
}

} // namespace detail
} // namespace graphics
} // namespace ad
