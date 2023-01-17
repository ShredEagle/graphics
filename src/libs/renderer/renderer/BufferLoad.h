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


} // namespace graphics
} // namespace ad
