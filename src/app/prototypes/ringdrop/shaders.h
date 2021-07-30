#pragma once

namespace ad {

const GLchar* gVertexShader = R"#(
    #version 400

    layout(location=0) in vec4 in_VertexPosition;
    layout(location=1) in vec2 in_UV;
    layout(location=2) in vec2 in_InstancePosition;
    layout(location=3) in float in_RotationPerSec;
    out vec2 ex_UV;
    out float ex_RotationPerSec;

    void main(void)
    {
        // Column major notation, which seems to be the convention in OpenGL
        mat4 transform = mat4(
            0.1, 0.0, 0.0, 0.0,
            0.0, 0.1, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0,
            in_InstancePosition.x, in_InstancePosition.y, 0.0, 1.0
        );
        gl_Position = transform*in_VertexPosition;
        ex_UV = in_UV;
        ex_RotationPerSec = in_RotationPerSec;
    }
)#";


const GLchar* gAnimationFragmentShader = R"#(
    #version 400

    in vec2 ex_UV;
    in float ex_RotationPerSec;
    out vec4 out_Color;
    uniform sampler2DArray spriteSampler;
    uniform float time;

    void main(void)
    {
        int frameCount = 8;
        int frame = int(ex_RotationPerSec * time * frameCount) % frameCount;
        out_Color = texture(spriteSampler, vec3(ex_UV, frame));
    }
)#";

} // namespace ad
