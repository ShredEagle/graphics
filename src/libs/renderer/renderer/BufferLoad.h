#pragma once

#include "BufferBase.h"
#include "ScopeGuards.h"

#include <span>


namespace ad {
namespace graphics {


template <class T_data, std::size_t N_extent, BufferType N_type>
void load(const Buffer<N_type> & aBuffer, std::span<T_data, N_extent> aData, BufferHint aUsageHint)
{
    ScopedBind bound{aBuffer};
    glBufferData(static_cast<GLenum>(N_type),
                 aData.size_bytes(),
                 aData.data(),
                 getGLBufferHint(aUsageHint));
}


/// @brief Updates a subset of a buffer data store.
/// @param aInstanceCount The offset to the start of the buffer subset to replace,
/// expressed in number of objects, **not** bytes.
/// @param aData 
template <class T_data, std::size_t N_extent, BufferType N_type>
void replaceSubset(const Buffer<N_type> & aBuffer, 
                   GLsizei aInstanceCount,
                   std::span<T_data, N_extent> aData)
{
    ScopedBind bound{aBuffer};
    glBufferSubData(static_cast<GLenum>(N_type),
                    sizeof(T_data) * aInstanceCount,
                    aData.size_bytes(),
                    aData.data());
}


/// @brief Initialize a data store of a size accomodationg `aInstanceCount` object of type `T_data`.
template <class T_data, BufferType N_type>
void initialize(const Buffer<N_type> & aBuffer, GLsizei aInstanceCount, BufferHint aUsageHint)
{
    ScopedBind bound{aBuffer};
    glBufferData(static_cast<GLenum>(N_type),
                 sizeof(T_data) * aInstanceCount,
                 NULL,
                 getGLBufferHint(aUsageHint));
}


} // namespace graphics
} // namespace ad
