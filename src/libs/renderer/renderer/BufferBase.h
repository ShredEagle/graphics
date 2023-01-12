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

/// \brief A non-owning version of the resource, storing a copy of the GLuint "name" of the resource.
/// The advantage over the plain GLuint is it preserves some strong typing.
/// \attention Buffer name in the sense of the "name" returned by glGenBuffers (which is an unsigned integer)
template <class T_resource>
class Name 
{
    friend Name<T_resource> getBound(const T_resource &);
    
    template <BufferType N_type>
    friend Name<Buffer<N_type>> getBound(const Buffer<N_type> &);

public:
    /*implicit*/ Name(const T_resource & aBuffer) :
        mResource{aBuffer}
    {}

    /*implicit*/ operator const GLuint() const
    {
        return mResource;
    }

private:
    explicit Name(GLuint aResource) :
        mResource{aResource}
    {}

    GLuint mResource;
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
    return Name<Buffer<N_type>>{(GLuint)current};
}


} // namespace graphics
} // namespace ad