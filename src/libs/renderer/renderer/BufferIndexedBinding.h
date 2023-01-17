#pragma once


#include "BufferBase.h"
#include "MappedGL.h"


namespace ad {
namespace graphics {


/// \brief Used to represent the location for indexed binding points.
struct BindingIndex
{
    explicit constexpr BindingIndex(GLuint aIndex) noexcept :
        mIndex{aIndex}
    {}

    /*implicit*/ constexpr operator GLuint () const noexcept
    { return mIndex; }

private:
    GLuint mIndex;
};


/// \brief Binding to an index location.
template <BufferType N_type>
void bind(const Buffer<N_type> & aBuffer, BindingIndex aIndex)
{
    glBindBufferBase(static_cast<GLenum>(N_type), aIndex, aBuffer);
}


template <BufferType N_type>
void bind(const Name<Buffer<N_type>> & aBuffer, BindingIndex aIndex)
{
    glBindBufferBase(static_cast<GLenum>(N_type), aIndex, aBuffer);
}


template <BufferType N_type>
Name<Buffer<N_type>> getBound(const Buffer<N_type> & aBuffer, BindingIndex aIndex)
{
    GLint current;
    glGetIntegeri_v(getGLMappedBufferBinding(static_cast<GLenum>(N_type)), aIndex, &current);
    return Name<Buffer<N_type>>{(GLuint)current, typename Name<Buffer<N_type>>::UnsafeTag{}};
}


} // namespace graphics
} // namespace ad
