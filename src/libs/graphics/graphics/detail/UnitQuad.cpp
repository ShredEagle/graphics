#include "UnitQuad.h"

#include "../shaders.h"

#include <renderer/Uniforms.h>


namespace ad {
namespace graphics {
namespace detail {


std::array<VertexUnitQuad, gQuadVerticeCount> make_RectangleVertices(math::Rectangle<GLfloat> aVertices)
{
    return std::array<VertexUnitQuad, gQuadVerticeCount>{
        VertexUnitQuad{
            aVertices.bottomLeft(),
            {0.0f, 0.0f},
        },
        VertexUnitQuad{
            aVertices.topLeft(),
            {0.0f,  1.0f},
        },
        VertexUnitQuad{
            aVertices.bottomRight(),
            { 1.0f, 0.0f},
        },
        VertexUnitQuad{
            aVertices.topRight(),
            { 1.0f,  1.0f},
        },
    };
}

VertexSpecification make_UnitQuad()
{
    return make_Rectangle({ {-1.f, -1.f}, {2.f, 2.f} });
}

VertexSpecification make_Rectangle(math::Rectangle<GLfloat> aVertices)
{
    std::array<VertexUnitQuad, gQuadVerticeCount> gVerticesScreen = make_RectangleVertices(aVertices);
    VertexSpecification result;
    appendToVertexSpecification(result, gVertexScreenDescription, std::span(gVerticesScreen));
    return result;
}

Program make_PassthroughProgram(GLint aTextureUnit)
{
    Program passthrough = makeLinkedProgram({
        {GL_VERTEX_SHADER, gPassthroughVertexShader},
        {GL_FRAGMENT_SHADER, gTexturingFragmentShader}
    });

    setUniform(passthrough, "inputTexture", aTextureUnit);

    return passthrough;
}



} // namespace detail
} // namespace graphics
} // namespace ad

