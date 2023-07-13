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


inline GLenum getEnum(GLenum aParameter)
{
    GLint data[4];
    glGetIntegerv(aParameter, data);
    return (GLenum)data[0]; 
}


inline GLint getInt(GLenum aParameter)
{
    GLint data;
    glGetIntegerv(aParameter, &data);
    return data;
}


} // namespace graphics
} // namespace ad
