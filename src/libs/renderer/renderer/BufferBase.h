#pragma once


#include "gl_helpers.h"
#include "GL_Loader.h"

#include <handy/Guard.h>


namespace ad {
namespace graphics {


template <BufferType N_type>
struct [[nodiscard]] Buffer : public ResourceGuard<GLuint>
{
    Buffer() :
        ResourceGuard<GLuint>{reserve(glGenBuffers),
                              [](GLuint aIndex){glDeleteBuffers(1, &aIndex);}}
    {}

};


template <BufferType N_type>
void bind(const Buffer<N_type> & aBuffer)
{
    glBindBuffer(static_cast<GLenum>(N_type), aBuffer);
}


template <BufferType N_type>
void bind(const Name<Buffer<N_type>> & aBufferView)
{
    glBindBuffer(static_cast<GLenum>(N_type), aBufferView);
}


template <BufferType N_type>
void unbind(const Buffer<N_type> &)
{
    glBindBuffer(static_cast<GLenum>(N_type), 0);
}


template <BufferType N_type>
Name<Buffer<N_type>> getBound(const Buffer<N_type> &)
{
    GLint current;
    glGetIntegerv(static_cast<GLenum>(N_type), &current);
    return Name<Buffer<N_type>>{(GLuint)current, typename Name<Buffer<N_type>>::UnsafeTag{}};
}


} // namespace graphics
} // namespace ad