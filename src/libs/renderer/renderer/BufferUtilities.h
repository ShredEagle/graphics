#pragma once


#include "MappedGL.h"
#include "UniformBuffer.h" // TODO remove when generalized
#include "ScopeGuards.h"

#include <span>


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
    glBindBufferBase(GL_UNIFORM_BUFFER, aIndex, aBuffer);
}


template <BufferType N_type>
void bind(const Name<Buffer<N_type>> & aBuffer, BindingIndex aIndex)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, aIndex, aBuffer);
}


template <BufferType N_type>
Name<Buffer<N_type>> getBound(const Buffer<N_type> & aBuffer, BindingIndex aIndex)
{
    GLint current;
    glGetIntegeri_v(getGLMappedBufferBinding(static_cast<GLenum>(N_type)), aIndex, &current);
    return Name<Buffer<N_type>>{(GLuint)current, typename Name<Buffer<N_type>>::UnsafeTag{}};
}


// TODO make generic, by having traits for all buffer types.
template <class T_data, std::size_t N_extent, BufferType N_type>
void load(const Buffer<N_type> & aBuffer, std::span<T_data, N_extent> aData, BufferHint aUsageHint)
{
    ScopedBind bound{aBuffer};
    glBufferData(GL_UNIFORM_BUFFER, aData.size_bytes(), aData.data(), getGLBufferHint(aUsageHint));
}


} // namespace graphics
} // namespace ad
