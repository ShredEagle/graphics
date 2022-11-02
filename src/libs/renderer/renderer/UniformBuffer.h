#pragma once


#include "gl_helpers.h"
#include "GL_Loader.h"

#include <handy/Guard.h>


namespace ad {
namespace graphics {


struct [[nodiscard]] UniformBufferObject : public ResourceGuard<GLuint>
{
    UniformBufferObject() :
        ResourceGuard<GLuint>{reserve(glGenBuffers),
                              [](GLuint aIndex){glDeleteBuffers(1, &aIndex);}}
    {}
};


inline void bind(const UniformBufferObject & aUniformBuffer)
{
    glBindBuffer(GL_UNIFORM_BUFFER, aUniformBuffer);
}


inline void unbind(const UniformBufferObject &)
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


inline GLenum getBound(const UniformBufferObject &)
{
    GLint current;
    glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &current);
    return current;
}


template <>
inline ScopedBind::ScopedBind(const UniformBufferObject & aResource) :
    mGuard{[previous = getBound(aResource)]
           { glBindBuffer(GL_UNIFORM_BUFFER_BINDING, previous);}}
{
    bind(aResource);
}


} // namespace graphics
} // namespace ad
