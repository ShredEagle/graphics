
#pragma once

#include "commons.h"
#include "gl_helpers.h"
#include "GL_Loader.h"

#include <handy/Guard.h>


namespace ad {
namespace graphics {


struct [[nodiscard]] Sampler : public ResourceGuard<GLuint>
{
    Sampler() :
        ResourceGuard<GLuint>{reserve(glGenSamplers),
                              [](GLuint samplerId){glDeleteSamplers(1, &samplerId);}}
    {}
};


template <>
class Name<Sampler> : public detail::NameBase<Sampler>
{
public:
    explicit Name(GLuint aResource, UnsafeTag) :
        NameBase{aResource, UnsafeTag{}}
    {}

    /*implicit*/ Name(const Sampler & aSampler) :
        NameBase{aSampler}
    {}
};


/// \brief Bind the texture to the currently active texture unit.
inline void bind(const Sampler & aSampler, GLint aTextureUnit)
{
    glBindSampler(aTextureUnit, aSampler);
}


inline void bind(Name<Sampler> aSampler, GLint aTextureUnit)
{
    glBindSampler(aTextureUnit, aSampler);
}


/// @note Sampler parameter is unused, but provided for consistency with our usual approach. 
inline void unbind(const Sampler &, GLint aTextureUnit)
{
    glBindSampler(aTextureUnit, 0);
}


inline Name<Sampler> getBound(const Sampler & aSampler, GLint aTextureUnit)
{
    // Will restore the previously active texture unit on exit.
    Guard scopedTextureUnit = scopeTextureUnitActivation(aTextureUnit);

    GLint current;
    glGetIntegerv(GL_SAMPLER_BINDING, &current);
    return Name<Sampler>{(GLuint)current, Name<Sampler>::UnsafeTag{}};
}


} // namespace graphics
} // namespace ad
