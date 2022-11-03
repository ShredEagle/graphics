#pragma once

#include "commons.h"

#include <renderer/commons.h>
#include <renderer/VertexSpecification.h>

namespace ad {
namespace graphics {

struct Vertex
{
    Vec2<GLfloat> mPosition;
    Vec2<GLint> mUV;
};


constexpr std::initializer_list<AttributeDescription> gVertexDescription = {
    { 0,                                2, offsetof(Vertex, mPosition), MappedGL<GLfloat>::enumerator},
    { {1, ShaderParameter::Access::Integer} , 2, offsetof(Vertex, mUV),       MappedGL<GLint>::enumerator},
};

} // namespace graphics
} // namespace ad
