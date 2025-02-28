#include "Dds.h"

#include "DdsEnums.h"

#include "../Logging.h"

#include <cassert>


namespace ad::arte::dds {

namespace {


    template <class T_destination>
    void readBytes(std::istream & aIn, T_destination & aDestination)
    {
        aIn.read(reinterpret_cast<char *>(&aDestination), sizeof(T_destination));
    }


    template <class T_destination>
    T_destination readBytes(std::istream & aIn)
    {
        T_destination destination;
        readBytes(aIn, destination);
        return destination;
    }


} // unnamed namespace


Header readHeader(std::istream & aDdsStream)
{
    // see: https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dx-graphics-dds-pguide#dds-file-layout
    std::uint32_t dwMagic;
    readBytes(aDdsStream, dwMagic);
    assert(dwMagic == 0x20534444); // "DDS ", little endian

    Header result;
    readBytes(aDdsStream, result.h);
    // Sanity checks
    assert((result.h.dwFlags & DDS_HEADER_FLAGS_TEXTURE) == DDS_HEADER_FLAGS_TEXTURE);
    assert((result.h.dwCaps & DDSCAPS_TEXTURE) == DDSCAPS_TEXTURE);

    if(((result.h.ddspf.dwFlags & DDPF_FOURCC) != 0)
        && result.h.ddspf.dwFourCC == 0x30315844) // "DX10", little endian
    {
        result.h_dxt10 = readBytes<DDS_HEADER_DXT10>(aDdsStream);
    }

    return result;
}


math::Size<2, unsigned int> getDimensions(const Header & aHeader)
{
    return {aHeader.h.dwWidth, aHeader.h.dwHeight};
}


} // namespace ad::arte::dds
