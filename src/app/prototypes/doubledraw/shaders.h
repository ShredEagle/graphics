#pragma once

namespace ad {
namespace graphics {

const GLchar* gVertexShader = R"#(
    #version 400

    layout(location=0) in vec4 in_Position;
    layout(location=1) in vec2 in_UV;
    out vec2 ex_UV;

    void main(void)
    {
        gl_Position = in_Position;
        ex_UV = in_UV;
    }
)#";

const GLchar* gFragmentShader = R"#(
    #version 400

    in vec2 ex_UV;
    out vec4 out_Color;
    // Only works starting with 4.2
    //layout(binding=1) uniform sampler2D spriteSampler;
    uniform sampler2D spriteSampler;

    void main(void)
    {
        out_Color = texture(spriteSampler, ex_UV);
    }
)#";

const GLchar* gAnimationFragmentShader = R"#(
    #version 400

    in vec4 ex_Color;
    in vec2 ex_UV;
    out vec4 out_Color;
    uniform sampler2DArray spriteSampler;
    uniform int frame;

    void main(void)
    {
        out_Color = texture(spriteSampler, vec3(ex_UV, frame));
    }
)#";


} // namespace graphics
} // namespace ad
