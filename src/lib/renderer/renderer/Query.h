#pragma once


#include "GL_Loader.h"


namespace ad {
namespace graphics {


inline int getMaxTextureSize()
{
    GLint result;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &result);
    return result;
}


} // namespace graphics
} // namespace ad
