#pragma once


#include "gl_helpers.h"
#include "GL_Loader.h"
#include "MappedGL.h"

#include <handy/Guard.h>


namespace ad {
namespace graphics {


enum class BufferType
{
    Array = GL_ARRAY_BUFFER,
    ElementArray = GL_ELEMENT_ARRAY_BUFFER,
    Uniform = GL_UNIFORM_BUFFER,
};


template <BufferType N_type>
struct [[nodiscard]] Buffer : public ResourceGuard<GLuint>
{
    static constexpr GLenum GLTarget_v = static_cast<GLenum>(N_type);

    struct NullTag{};

    Buffer() :
        ResourceGuard<GLuint>{reserve(glGenBuffers),
                              [](GLuint aIndex){glDeleteBuffers(1, &aIndex);}}
    {}

    Buffer(NullTag) :
        ResourceGuard<GLuint>{0,
                              [](GLuint aIndex){}}
    {}
};


/// @brief RAII wrapper around GL buffers, without coupling to any specific target.
/// @note Buffers can actually be bound to different targets during their lifetime.
/// glspec4.6 6.1: 
/// > Buffer objects created by binding a name returned by GenBuffers to any 
/// > of the valid targets are formally equivalent.
struct [[nodiscard]] BufferAny : public ResourceGuard<GLuint>
{
    BufferAny() :
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
void bind(Name<Buffer<N_type>> aBufferView)
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
    glGetIntegerv(getGLMappedBufferBinding(static_cast<GLenum>(N_type)), &current);
    return Name<Buffer<N_type>>{(GLuint)current, typename Name<Buffer<N_type>>::UnsafeTag{}};
}


inline void bind(const BufferAny & aBuffer, BufferType aTarget)
{
    glBindBuffer(static_cast<GLenum>(aTarget), aBuffer);
}


inline void bind(Name<BufferAny> aBufferView, BufferType aTarget)
{
    glBindBuffer(static_cast<GLenum>(aTarget), aBufferView);
}


inline Name<BufferAny> getBound(const BufferAny &, BufferType aTarget)
{
    GLint current;
    glGetIntegerv(getGLMappedBufferBinding(static_cast<GLenum>(aTarget)), &current);
    return Name<BufferAny>{(GLuint)current, typename Name<BufferAny>::UnsafeTag{}};
}



} // namespace graphics
} // namespace ad
