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


inline bool isEnabled(GLenum aTestedFeature)
{
    GLboolean result;
    glGetBooleanv(aTestedFeature, &result);
    return result;
}


} // namespace graphics
} // namespace ad
