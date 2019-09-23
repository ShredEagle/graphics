#pragma once

#include "commons.h"

#include <renderer/VertexSpecification.h>

namespace ad {

struct Vertex
{
    math::Vec2<GLfloat> mPosition;
    math::Vec2<GLint> mUV;
};


const std::vector<const AttributeDescription> gVertexDescription = {
    {0, 2, offsetof(Vertex, mPosition), MappedGL<GLfloat>::enumerator},
    {1, 2, offsetof(Vertex, mUV),       MappedGL<GLint>::enumerator, ShaderAccess::Integer},
};

} // namespace ad
