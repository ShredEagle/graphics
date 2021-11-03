#include "UnitQuad.h"

#include "../shaders.h"

#include <renderer/Uniforms.h>


namespace ad {
namespace graphics {
namespace detail {


VertexSpecification make_UnitQuad()
{
    std::array<VertexUnitQuad, 4> gVerticesScreen = {
        VertexUnitQuad{
            {-1.0f, -1.0f},
            {0.0f, 0.0f},
        },
        VertexUnitQuad{
            {-1.0f,  1.0f},
            {0.0f,  1.0f},
        },
        VertexUnitQuad{
            { 1.0f, -1.0f},
            { 1.0f, 0.0f},
        },
        VertexUnitQuad{
            { 1.0f,  1.0f},
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

