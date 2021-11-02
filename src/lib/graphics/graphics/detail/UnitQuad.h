#pragma once


#include <renderer/commons.h>
#include <renderer/Shading.h>
#include <renderer/VertexSpecification.h>

#include <glad/glad.h>


namespace ad {
namespace graphics {
namespace detail {


struct VertexUnitQuad
{
    Vec2<GLfloat> mPosition;
    Vec2<GLfloat> mUV;
};


constexpr std::initializer_list<AttributeDescription> gVertexScreenDescription = {
    { 0, 2, offsetof(VertexUnitQuad, mPosition), MappedGL<GLfloat>::enumerator},
    { 1, 2, offsetof(VertexUnitQuad, mUV),       MappedGL<GLfloat>::enumerator},
};


// TODO make a global VertexSpecification instance instead
VertexSpecification make_UnitQuad();

/// \brief Notably usefull to copy from one framebuffer to another framebuffer
/// using a quad covering the whole buffer.
Program make_PassthroughProgram();


} // namespace detail
} // namespace graphics
} // namespace ad

