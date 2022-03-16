#pragma once


namespace ad {
namespace gltfviewer {


inline const GLchar* gNaiveVertexShader = R"#(
    #version 400

    layout(location=0) in vec4 ve_position;
    layout(location=1) in vec3 ve_normal;

    layout(location=8) in mat4 in_modelTransform;

    uniform mat4 u_camera;
    uniform mat4 u_projection;

    void main(void)
    {
        gl_Position = u_projection * u_camera * in_modelTransform * ve_position;
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
