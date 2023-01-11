#pragma once


#include "MappedGL.h"
#include "UniformBuffer.h" // TODO remove when generalized

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


// TODO make generic for all indexed buffer types.
/// \brief Binding to an index location.
inline void bind(const UniformBufferObject & aUniformBuffer, BindingIndex aIndex)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, aIndex, aUniformBuffer);
}


// TODO make generic, by having traits for all buffer types.
template <class T_data, std::size_t N_extent>
inline void load(const UniformBufferObject & aBuffer, std::span<T_data, N_extent> aData, BufferHint aUsageHint)
{
    ScopedBind bound{aBuffer};
    glBufferData(GL_UNIFORM_BUFFER, aData.size_bytes(), aData.data(), getGLBufferHint(aUsageHint));
}


} // namespace graphics
} // namespace ad
