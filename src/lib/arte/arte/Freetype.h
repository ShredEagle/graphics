#pragma once 


#include <handy/Guard.h>

#include <platform/Filesystem.h>

#include <math/Vector.h>

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H // For bbox measurements

#include <exception>
#include <iostream>


namespace ad {
namespace arte {

#define GENERIC_LOGICERROR(fterror) \
    std::logic_error{"Error " + std::to_string(fterror) + \
                     " in " + __func__ + "(" + std::to_string(__LINE__) + ")."}


using CharCode = std::uint32_t;


class FontFace;


/// \brief Wrap the FreeType library instance, which must exist while any 
/// subobject is still alive.
class Freetype : public ResourceGuard<FT_Library>
{
public:
    Freetype() :
        ResourceGuard<FT_Library>{initialize(), &FT_Done_FreeType}
    {}

    FontFace load(const filesystem::path & aFontPath) const;

private:
    static FT_Library initialize()
    {
        FT_Library library;
        FT_Error error = FT_Init_FreeType(&library);
        if (error)
        {
            throw std::runtime_error{"Unable to initialize freetype, error: " + std::to_string(error)};
        }
        return library;
    }
};


class GlyphSlot;

/// \brief Wrap the bitmap for a specific glyph.
///
/// \warning The FontFace instance must outlive the GlyphBitmap
///
/// \note The GlyphSlot does not need to outlive the GlyphSlot.
/// It is actually kept alive in the FontFace as long as the glyph is not changed with 
/// `FontFace::getGlyph()`.
class GlyphBitmap
{
    friend class GlyphSlot;
    friend class Glyph;
public:
    int width() const
    {
        return mBitmap.width;
    }

    int rows() const
    {
        return mBitmap.rows;
    }

    std::size_t bytesize() const
    {
        // The pitch's absolute value is the number of bytes taken by one bitmap row.
        return std::abs(mBitmap.pitch) * rows();
    }

    const std::byte * data() const
    {
        return reinterpret_cast<const std::byte *>(mBitmap.buffer);
    }

private:
    GlyphBitmap(FT_Bitmap aBitmap) :
        mBitmap{std::move(aBitmap)}
    {};

    FT_Bitmap mBitmap;
};



class GlyphSlot;


class Glyph : public ResourceGuard<FT_Glyph>
{
    friend class GlyphSlot;

public:
    math::Size<2, int> getBoundingDimension()
    {
        FT_BBox bbox;
        //FT_Glyph_Get_CBox(*this, FT_GLYPH_BBOX_TRUNCATE, &bbox);
        FT_Glyph_Get_CBox(*this, FT_GLYPH_BBOX_PIXELS, &bbox);
        return {
            bbox.xMax - bbox.xMin,
            bbox.yMax - bbox.yMin
        };
    }

    GlyphBitmap render()
    {
        FT_Glyph resource{get()};
        if (FT_Error error = FT_Glyph_To_Bitmap(&resource,
                                                FT_RENDER_MODE_NORMAL,
                                                nullptr/*offset*/,
                                                0/*do **not** destroy input glyph, resetResource() will*/))
        {
            throw GENERIC_LOGICERROR(error);
        }
        resetResource(resource, &FT_Done_Glyph);
        return ((FT_BitmapGlyph)get())->bitmap;
    }

private:
    Glyph(FT_Glyph aGlyph) :
        ResourceGuard<FT_Glyph>{aGlyph, &FT_Done_Glyph}
    {}
};


// Note: GlyphSlot might be copied to get value semantic, with
// the instance becoming independant of the FontFace and its currently active glyph.
// Yet, we keep it minimal (and optimal) as the current use case can live with the limitations.
/// \brief Wrap the glyph slot currently active in a FontFace.
/// \warning The FontFace instance must outlive the GlyphBitmap.
class GlyphSlot
{
    friend class FontFace;

public:
    GlyphBitmap render() const
    {
        if (FT_Error error = FT_Render_Glyph(mGlyphSlot, FT_RENDER_MODE_NORMAL))
        {
            throw GENERIC_LOGICERROR(error);
        }
        return GlyphBitmap{mGlyphSlot->bitmap};
    }

    FT_Glyph_Metrics metric() const
    {
        return mGlyphSlot->metrics;
    }

    FT_UInt index() const
    {
        return mGlyphIndex;
    }

    Glyph getGlyph() const
    {
        FT_Glyph glyph;
        FT_Get_Glyph(mGlyphSlot, &glyph);
        return Glyph{glyph};
    }


private:
    GlyphSlot(FT_GlyphSlot aGlyph, FT_UInt aGlyphIndex) :
        mGlyphSlot{std::move(aGlyph)},
        mGlyphIndex{aGlyphIndex}
    {};

    FT_GlyphSlot mGlyphSlot;
    FT_UInt mGlyphIndex;
};


/// \brief Wrap a font face.
/// \warning The library object must outlive the FontFace.
class FontFace : public ResourceGuard<FT_Face>
{
public:
    FontFace(const Freetype & aLibrary, const filesystem::path & aFontPath) :
        ResourceGuard<FT_Face>{initialize(aLibrary, aFontPath), &FT_Done_Face}
    {}


    FontFace & inverseYAxis(bool aInverse)
    {
        static FT_Matrix inversion {1<<16, 0, 0, -(1<<16)};
        if (aInverse)
        {
            FT_Set_Transform(*this, &inversion, nullptr);
        }
        else
        {
            FT_Set_Transform(*this, nullptr, nullptr);
        }
        return *this;
    }

    /// \brief Define the pixel size of the GlyphBitmaps that will be obtained via `getGlyph()`.
    FontFace & setPixelSize(math::Size<2, int> aSize)
    {
        if (FT_Error error = FT_Set_Pixel_Sizes(*this, aSize.width(), aSize.height()))
        {
            throw GENERIC_LOGICERROR(error);
        }
        return *this;
    }

    FontFace & setPixelHeight(int aHeight)
    {
        return setPixelSize({0, aHeight});
    }

    bool hasGlyph(CharCode aCharcode) const
    {
        // Explicit conversion to check it cannot lose precision.
        return FT_Get_Char_Index(*this, FT_ULong{aCharcode}) != 0;
    }

    GlyphSlot getGlyphSlot(FT_ULong aCharcode) const
    {
        FT_UInt glyphIndex = FT_Get_Char_Index(*this, aCharcode);
        if (glyphIndex == 0)
        {
            throw std::invalid_argument{"Could not find a glyph for code: " + std::to_string(aCharcode)};
        }

        if (FT_Error error = FT_Load_Glyph(*this, glyphIndex, /*FT_LOAD_NO_BITMAP*/0))
        {
            throw GENERIC_LOGICERROR(error);
        }

        return GlyphSlot{get()->glyph, glyphIndex};
    }

    math::Vec<2, float> kern(FT_UInt aLeftGlyphIndex, FT_UInt aRightGlyphIndex)
    {
        FT_Vector kerning;
        FT_Get_Kerning(*this, aLeftGlyphIndex, aRightGlyphIndex, FT_KERNING_DEFAULT, &kerning);
        return {
            (float)kerning.x / (1 << 6),
            (float)kerning.y / (1 << 6) 
        };
    }
        

private:
    static FT_Face initialize(FT_Library aLibrary, const filesystem::path & aFontPath)
    {
        FT_Face face;
        FT_Error error = FT_New_Face(aLibrary, aFontPath.string().c_str(), 0, &face);
        if ( error == FT_Err_Unknown_File_Format )
        {
            throw std::invalid_argument{"Font file format appears not to be supported: " + aFontPath.string()};
        }
        else if ( error )
        {
            throw std::logic_error{"Error while accessing the font file: " + aFontPath.string()};
        }
        if (face->charmap == NULL)
        {
            throw std::logic_error{"Loaded font face does not have a default charmap selected."};
        }
        return face;
    }
};


//
// Implementations
//
inline FontFace Freetype::load(const filesystem::path & aFontPath) const
{
    return FontFace{*this, aFontPath};
}


#undef GENERIC_LOGICERROR


} // namespace arte
} // namespace ad
