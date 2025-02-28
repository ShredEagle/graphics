#pragma once


#include "GL_Loader.h"

#include <arte/dds/Dds.h>


namespace ad {
namespace graphics {


    GLenum getCompressedFormat(const arte::dds::Header & aHeader);

    GLsizei getCompressedByteSize(const arte::dds::Header & aHeader);

    GLenum getTextureTarget(const arte::dds::Header & aHeader);


} // namespace graphics
} // namespace ad
