#pragma once

#include "BufferBase.h"
#include "ScopeGuards.h"

#include <span>


namespace ad {
namespace graphics {


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


template <class T_data, std::size_t N_extent, BufferType N_type>
void load(const Buffer<N_type> & aBuffer, std::span<T_data, N_extent> aData, BufferHint aUsageHint)
{
    ScopedBind bound{aBuffer};
    glBufferData(static_cast<GLenum>(N_type),
                 aData.size_bytes(),
                 aData.data(),
                 getGLBufferHint(aUsageHint));
}


template <class T_data, std::size_t N_extent>
void load(const BufferAny & aBuffer, std::span<T_data, N_extent> aData, BufferHint aUsageHint)
{
    // TODO replace with DSA
    // Note: Waiting for DSA, use a random target, the underlying buffer objects are all identical.
    constexpr auto target = BufferType::Array;
    ScopedBind bound{aBuffer, target};
    glBufferData(static_cast<GLenum>(target),
                 aData.size_bytes(),
                 aData.data(),
                 getGLBufferHint(aUsageHint));
}


template <class T_data, BufferType N_type>
void loadSingle(const Buffer<N_type> & aBuffer, T_data aInstanceData, BufferHint aUsageHint)
{
    load(aBuffer, std::span{&aInstanceData, 1}, aUsageHint);
}


template <class T_data>
void loadSingle(const BufferAny & aBuffer, T_data aInstanceData, BufferHint aUsageHint)
{
    load(aBuffer, std::span{&aInstanceData, 1}, aUsageHint);
}


/// @brief Updates a subset of a buffer data store.
/// @param aInstanceCountOffset The offset to the start of the buffer subset to replace,
/// expressed in number of objects, **not** bytes.
/// @param aData 
template <class T_data, std::size_t N_extent, BufferType N_type>
void replaceSubset(const Buffer<N_type> & aBuffer, 
                   GLsizei aInstanceCountOffset,
                   std::span<T_data, N_extent> aData)
{
    ScopedBind bound{aBuffer};
    glBufferSubData(static_cast<GLenum>(N_type),
                    sizeof(T_data) * aInstanceCountOffset,
                    aData.size_bytes(),
                    aData.data());
}


template <class T_data, std::size_t N_extent>
void replaceSubset(const BufferAny & aBuffer, 
                   GLsizei aInstanceCountOffset,
                   std::span<T_data, N_extent> aData)
{
    // TODO replace with DSA
    // Note: Waiting for DSA, use a random target, the underlying buffer objects are all identical.
    constexpr auto target = BufferType::Array;
    ScopedBind bound{aBuffer, target};
    glBufferSubData(static_cast<GLenum>(target),
                    sizeof(T_data) * aInstanceCountOffset,
                    aData.size_bytes(),
                    aData.data());
}

} // namespace graphics
} // namespace ad
