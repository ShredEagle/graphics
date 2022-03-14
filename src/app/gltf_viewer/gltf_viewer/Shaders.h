#pragma once


namespace ad {
namespace gltfviewer {


inline const GLchar* gNaiveVertexShader = R"#(
    #version 400

    layout(location=0) in vec4 ve_position;
    layout(location=1) in vec3 ve_normal;

    uniform mat4 u_camera;
    uniform mat4 u_projection;
    uniform mat4 u_model;

    void main(void)
    {
        gl_Position = u_projection * u_camera * u_model * ve_position;
    }
)#";


inline const GLchar* gNaiveFragmentShader = R"#(
    #version 400

    out vec4 out_color;

    void main(void)
    {
        out_color = vec4(0.8, 0.8, 0.8, 1.0);
    }
)#";

} // namespace gltfviewer
} // namespace ad
