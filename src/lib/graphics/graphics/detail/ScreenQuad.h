#pragma once


#include <renderer/commons.h>
#include <renderer/Shading.h>
#include <renderer/VertexSpecification.h>

#include <glad/glad.h>


namespace ad {
namespace graphics {
namespace detail {


struct VertexScreenQuad
{
    Vec2<GLfloat> mPosition;
    Vec2<GLfloat> mUV;
};


constexpr std::initializer_list<AttributeDescription> gVertexScreenDescription = {
    { 0, 2, offsetof(VertexScreenQuad, mPosition), MappedGL<GLfloat>::enumerator},
    { 1, 2, offsetof(VertexScreenQuad, mUV),       MappedGL<GLfloat>::enumerator},
};


// TODO make a global VertexSpecification instance instead
VertexSpecification make_ScreenQuad();

Program make_PassthroughProgram();


} // namespace detail
} // namespace graphics
} // namespace ad

