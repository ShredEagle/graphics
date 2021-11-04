#include "UnitQuad.h"

#include "../shaders.h"

#include <renderer/Uniforms.h>


namespace ad {
namespace graphics {
namespace detail {


VertexSpecification make_UnitQuad()
{
    return make_Rectangle({ {-1.f, -1.f}, {2.f, 2.f} });
}

VertexSpecification make_Rectangle(math::Rectangle<GLfloat> aVertices)
{
    std::array<VertexUnitQuad, 4> gVerticesScreen = {
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

    VertexSpecification result;
    appendToVertexSpecification(result, gVertexScreenDescription, gsl::make_span(gVerticesScreen));
    return result;
}

Program make_PassthroughProgram(GLint aTextureUnit)
{
    Program passthrough = makeLinkedProgram({
        {GL_VERTEX_SHADER, gPassthroughVertexShader},
        {GL_FRAGMENT_SHADER, gTexturingFragmentShader}
    });

    setUniformInt(passthrough, "inputTexture", aTextureUnit);

    return passthrough;
}



} // namespace detail
} // namespace graphics
} // namespace ad

