#pragma once

namespace ad {

const GLchar* gVertexShader = R"#(
    #version 400

    layout(location=0) in vec4 in_VertexPosition;
    layout(location=1) in ivec2 in_UV;
    layout(location=2) in vec2 in_InstancePosition;
    layout(location=3) in ivec4 in_TextureArea;

    uniform ivec2 in_BufferResolution;
    
    out vec2 ex_UV;

    void main(void)
    {
        //// Column major notation, which seems to be the convention in OpenGL
        //mat4 transform = mat4(
        //    1.0, 0.0, 0.0, 0.0,
        //    0.0, 1.0, 0.0, 0.0,
        //    0.0, 0.0, 0.0, 0.0,
        //    in_InstancePosition.x, in_InstancePosition.y, 0.0, 1.0
        //);
        //gl_Position = clipTransform * (transform * in_VertexPosition);

        vec2 bufferSpacePosition = in_InstancePosition + in_VertexPosition.xy * in_TextureArea.zw;
        gl_Position = vec4(2 * bufferSpacePosition / in_BufferResolution - vec2(1.0, 1.0),
                           0.0, 1.0);

        ex_UV = in_TextureArea.xy + in_UV*in_TextureArea.zw;
    }
)#";


const GLchar* gAnimationFragmentShader = R"#(
    #version 400

    in vec2 ex_UV;
    out vec4 out_Color;
    uniform sampler2DRect spriteSampler;

    void main(void)
    {
        out_Color = texture(spriteSampler, ex_UV);
    }
)#";

} // namespace ad
