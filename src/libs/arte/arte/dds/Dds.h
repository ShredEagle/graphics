#pragma once


#include <math/Vector.h>

#include <istream>
#include <optional>

#include <cstdint>


// Handle dds file format
// see: https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dx-graphics-dds-pguide

namespace ad::arte {


using Dword_t = std::uint32_t;


// Structures are copied directly from microsoft documentation.

// see: https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-pixelformat
struct DDS_PIXELFORMAT
{
    Dword_t dwSize;
    Dword_t dwFlags;
    Dword_t dwFourCC;
    Dword_t dwRGBBitCount;
    Dword_t dwRBitMask;
    Dword_t dwGBitMask;
    Dword_t dwBBitMask;
    Dword_t dwABitMask;
};


enum PixelFormat_DwFlags : Dword_t
{
    DDPF_ALPHAPIXELS = 0x1,
    DDPF_ALPHA = 0x2,
    DDPF_FOURCC = 0x4,
    DDPF_RGB = 0x40,
    DDPF_YUV = 0x200,
    DDPF_LUMINANCE = 0x20000,
};


// see: https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-header
struct DDS_HEADER
{
    Dword_t           dwSize;
    Dword_t           dwFlags;
    Dword_t           dwHeight;
    Dword_t           dwWidth;
    Dword_t           dwPitchOrLinearSize;
    Dword_t           dwDepth;
    Dword_t           dwMipMapCount;
    Dword_t           dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    Dword_t           dwCaps;
    Dword_t           dwCaps2;
    Dword_t           dwCaps3;
    Dword_t           dwCaps4;
    Dword_t           dwReserved2;
};


enum DdsHeader_DwFlags : Dword_t
{
    DDSD_CAPS = 0x1,
    DDSD_HEIGHT = 0x2,
    DDSD_WIDTH = 0x4,
    DDSD_PITCH = 0x8,
    DDSD_PIXELFORMAT = 0x1000,
    DDSD_MIPMAPCOUNT = 0x20000,
    DDSD_LINEARSIZE = 0x80000,
    DDSD_DEPTH = 0x800000,
};


inline Dword_t DDS_HEADER_FLAGS_TEXTURE =
    DDSD_CAPS
    | DDSD_HEIGHT
    | DDSD_WIDTH
    | DDSD_PIXELFORMAT;


// see: https://learn.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
using DXGI_FORMAT = std::uint32_t; // an enum whose max value is named FORCE_UINT, with a widht of 32 bits.


// see: https://learn.microsoft.com/en-us/windows/win32/api/d3d10/ne-d3d10-d3d10_resource_dimension
using D3D10_RESOURCE_DIMENSION = std::uint32_t; // an enum, that should map to an int, but everything else seems uint32_t


// see: https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-dtyp/52ddd4c3-55b9-4e03-8287-5392aac0627f
using UINT = std::uint32_t;


// see: https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-header-dxt10
struct DDS_HEADER_DXT10
{
  DXGI_FORMAT              dxgiFormat;
  D3D10_RESOURCE_DIMENSION resourceDimension;
  UINT                     miscFlag;
  UINT                     arraySize;
  UINT                     miscFlags2;
};


enum DdsHeaderDxt10_ResourceDimension : D3D10_RESOURCE_DIMENSION
{
    DDS_DIMENSION_TEXTURE1D = 2,
    DDS_DIMENSION_TEXTURE2D = 3,
    DDS_DIMENSION_TEXTURE3D = 4,
};


namespace dds {


    struct Header
    {
        DDS_HEADER h;
        std::optional<DDS_HEADER_DXT10> h_dxt10;
    };

    /// @param aDdsStream: an input stream to the beginning DDS data.
    /// When this function return, the stream will point at the beginning of the main surface data
    /// (the actual texture payload).
    Header readHeader(std::istream & aDdsStream);

    math::Size<2, unsigned int> getDimensions(const Header & aHeader);

} // namespace dds



} // namespace ad::arte
