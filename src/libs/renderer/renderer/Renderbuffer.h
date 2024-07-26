#pragma once

#include "gl_helpers.h"
//#include "ScopeGuards.h"

#include <handy/Guard.h>


namespace ad {
namespace graphics {


struct [[nodiscard]] Renderbuffer : public ResourceGuard<GLuint>
{
    Renderbuffer() :
        ResourceGuard<GLuint>(reserve(glGenRenderbuffers),
                              [](GLuint rbId){glDeleteRenderbuffers(1, &rbId);})
    {}
};


inline void bind(const Renderbuffer & aRenderbuffer)
{
    glBindRenderbuffer(GL_RENDERBUFFER, aRenderbuffer);
}


inline void bind(Name<Renderbuffer> aRenderbuffer)
{
    glBindRenderbuffer(GL_RENDERBUFFER, aRenderbuffer);
}


inline void unbind(const Renderbuffer & /*aRenderbuffer*/)
{
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}


inline Name<Renderbuffer> getBound(const Renderbuffer &)
{
    GLint current;
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &current);
    return Name<Renderbuffer>{(GLuint)current, Name<Renderbuffer>::UnsafeTag{}};
}


} // namespace graphics
} // namespace ad