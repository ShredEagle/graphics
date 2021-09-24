#pragma once


#include <glad/glad.h>


namespace ad {

const GLchar* gLineVertexShader = R"#(
    #version 400

    layout(location=0) in vec2 in_Position;

    void main(void)
    {
        gl_Position = vec4(in_Position, 0.0, 1.0);
    }
)#";


const GLchar* gGenerativeVertexShader = R"#(
    #version 400

    layout(location=0) in float in_t;
    layout(location=1) in int in_side;

    layout(location=2) in vec2 p0;
    layout(location=3) in vec2 p1;
    layout(location=4) in vec2 p2;
    layout(location=5) in vec2 p3;

    void main(void)
    {
        vec2 ab = (1-in_t) * p0 + in_t * p1;
        vec2 bc = (1-in_t) * p1 + in_t * p2;
        vec2 cd = (1-in_t) * p2 + in_t * p3;

        vec2 ac = (1-in_t) * ab + in_t * bc;
        vec2 bd = (1-in_t) * bc + in_t * cd;

        vec2 ad = (1-in_t) * ac + in_t * bd;

        gl_Position = vec4(ad, 0., 1.);
    }
)#";


const GLchar* gFragmentShader = R"#(
    #version 400

    out vec4 out_Color;

    void main(void)
    {
        out_Color = vec4(1.0, 0.0, 1.0, 1.0);
    }
)#";


} // namespace ad
