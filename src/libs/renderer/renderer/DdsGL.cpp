#include "DdsGL.h"

#include <arte/dds/DdsEnums.h>

#include <cassert>


namespace ad {
namespace graphics {


using namespace arte;


GLenum getCompressedFormat(const dds::Header & aHeader)
{
    if(aHeader.h_dxt10)
    {
        const DDS_HEADER_DXT10 & dxt10 = *aHeader.h_dxt10;
        switch(dxt10.dxgiFormat)
        {
            default:
                // TODO Ad 2024/07/24: Extend to support a reasonable set of formats.
                //ADLOG(error)("DXGI format {} is not supported at the moment", dxt10.dxgiFormat)
                throw std::domain_error("The texture format in this DDS is not supported at the moment.");
            case DXGI_FORMAT_BC5_UNORM:
                return GL_COMPRESSED_RG_RGTC2;
            case DXGI_FORMAT_BC5_SNORM:
                return GL_COMPRESSED_SIGNED_RG_RGTC2;
            case DXGI_FORMAT_BC6H_UF16:
                return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
            case DXGI_FORMAT_BC6H_SF16:
                return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
            case DXGI_FORMAT_BC7_UNORM:
                return GL_COMPRESSED_RGBA_BPTC_UNORM;
            case DXGI_FORMAT_BC7_UNORM_SRGB:
                return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;
        }
    }
    throw std::invalid_argument("This DDS does not contain an extended DXT10 header.");
}


GLsizei getCompressedByteSize(const dds::Header & aHeader)
{
    if ((aHeader.h.dwFlags & DDSD_LINEARSIZE) != 0)
    {
        return aHeader.h.dwPitchOrLinearSize;
    }

    throw std::domain_error("The texture in this DDS does not provide its linear size.");
}


GLenum getTextureTarget(const dds::Header & aHeader)
{
    if((aHeader.h.dwCaps2 & DDSCAPS2_CUBEMAP) == DDSCAPS2_CUBEMAP)
    {
        assert((aHeader.h.dwCaps & DDSCAPS_COMPLEX) == DDSCAPS_COMPLEX);
        if(aHeader.h.dwCaps2 == DDS_CUBEMAP_ALLFACES)
        {
            return GL_TEXTURE_CUBE_MAP;
        }
        else
        {
            //ADLOG(critical)("Partial cube-maps are not supported.");
            throw std::invalid_argument{"Partial cube-maps are not supported."};
        }
    }
    else if((aHeader.h.dwCaps2 & DDSCAPS2_VOLUME) == DDSCAPS2_VOLUME)
    {
        assert((aHeader.h.dwFlags & DDSD_DEPTH) == DDSD_DEPTH);
        return GL_TEXTURE_3D;
    }
    else if(aHeader.h_dxt10)
    {
        switch(aHeader.h_dxt10->resourceDimension)
        {
            default:
                throw std::logic_error{"Unknown resource dimension in DXT10 header."};
            case DDS_DIMENSION_TEXTURE1D:
                return GL_TEXTURE_1D;
            case DDS_DIMENSION_TEXTURE2D:
                return GL_TEXTURE_2D;
            case DDS_DIMENSION_TEXTURE3D:
                return GL_TEXTURE_3D;
        }
    }
    else
    {
        // Sanity check: there is no depth provided
        // (if so, it should have been marked as volume texture in CAPS2)
        assert((aHeader.h.dwFlags & DDSD_DEPTH) != DDSD_DEPTH);
        // I am not sure if this is how non-DXT10 1D texture are detected, so assert until it happens
        assert(aHeader.h.dwHeight > 1);
        return GL_TEXTURE_2D;
    }
}


} // namespace graphics
} // namespace ad
