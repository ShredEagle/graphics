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
    Position2<GLfloat> mPosition;
    Vec2<GLfloat> mUV;
};


constexpr std::initializer_list<AttributeDescription> gVertexScreenDescription = {
    { 0, {2, offsetof(VertexUnitQuad, mPosition), MappedGL<GLfloat>::enumerator}},
    { 1, {2, offsetof(VertexUnitQuad, mUV),       MappedGL<GLfloat>::enumerator}},
};


constexpr GLsizei gQuadVerticeCount = 4;

std::array<VertexUnitQuad, gQuadVerticeCount> make_RectangleVertices(math::Rectangle<GLfloat> aVertices);

VertexSpecification make_Rectangle(math::Rectangle<GLfloat> aVertices);
// TODO make a global VertexSpecification instance instead
VertexSpecification make_UnitQuad();

/// \brief Notably usefull to copy from one framebuffer to another framebuffer
/// using a quad covering the whole buffer.
/// \important The input texture will be read from texture unit `aTextureUnit`.
Program make_PassthroughProgram(GLint aTextureUnit = 0);


} // namespace detail
} // namespace graphics
} // namespace ad

