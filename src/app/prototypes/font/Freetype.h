#pragma once 


#include <handy/Guard.h>

#include <platform/Filesystem.h>

#include <math/Vector.h>

#include "ft2build.h"
#include FT_FREETYPE_H

#include <exception>
#include <iostream>


namespace ad {
namespace graphics {

#define GENERIC_LOGICERROR(fterror) \
    std::logic_error{"Error " + std::to_string(fterror) + \
                     " in " + __func__ + "(" + std::to_string(__LINE__) + ")."}

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


/// \brief Wrap the bitmap for a specific glyph.
/// \warning The FontFace instance must outlive the GlyphBitmap.
class GlyphBitmap
{
    friend class FontFace;
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

    const unsigned char * data() const
    {
        return mBitmap.buffer;
    }

private:
    GlyphBitmap(FT_Bitmap aBitmap) :
        mBitmap{std::move(aBitmap)}
    {};

    FT_Bitmap mBitmap;
};


/// \brief Wrap a font face.
/// \warning The library object must outlive the FontFace.
class FontFace : public ResourceGuard<FT_Face>
{
public:
    FontFace(const Freetype & aLibrary, const filesystem::path & aFontPath) :
        ResourceGuard<FT_Face>{initialize(aLibrary, aFontPath), &FT_Done_Face}
    {}

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

    GlyphBitmap getGlyph(FT_ULong aCharcode) const
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

        if (FT_Error error = FT_Render_Glyph(mResource->glyph, FT_RENDER_MODE_NORMAL))
        {
            throw GENERIC_LOGICERROR(error);
        }
        return GlyphBitmap{mResource->glyph->bitmap};
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
FontFace Freetype::load(const filesystem::path & aFontPath) const
{
    return FontFace{*this, aFontPath};
}



} // namespace graphics
} // namespace ad
