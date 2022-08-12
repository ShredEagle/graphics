#pragma once


#include <glad/glad.h>


namespace ad {

const GLchar* gVertexShader = R"#(
    #version 400

    layout(location=0) in vec2 in_Position;

    void main(void)
    {
        gl_Position = vec4(in_Position, 0.0, 1.0);
    }
)#";

const GLchar* gFragmentShader = R"#(
    #version 400

    out vec4 out_Color;

    void main(void)
    {
        out_Color = vec4(1.0, 1.0, 0.0, 1.0);
    }
)#";


} // namespace ad
