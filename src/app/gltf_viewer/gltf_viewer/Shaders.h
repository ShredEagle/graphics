#pragma once


namespace ad {
namespace gltfviewer {


inline const GLchar* gNaiveVertexShader = R"#(
    #version 400

    layout(location=0) in vec4 ve_Position;
    layout(location=1) in vec3 ve_Normal;

    void main(void)
    {
        gl_Position = ve_Position;
    }
)#";


inline const GLchar* gNaiveFragmentShader = R"#(
    #version 400

    out vec4 out_Color;

    void main(void)
    {
        out_Color = vec4(0.8, 0.8, 0.8, 1.0);
    }
)#";

} // namespace gltfviewer
} // namespace ad
