#pragma once


namespace ad {
namespace graphics {
namespace r2d {
namespace ShapingShaders {


const std::string gVertexShader = R"#(
#version 400

layout(location=0) in vec4  ve_Position_model;
layout(location=1) in vec4  in_Position_world;
layout(location=2) in vec2  in_Scale_world;

out vec4 ex_Color;

layout(std140) uniform ViewingBlock
{
    mat4 u_Camera;
    mat4 u_Projection;
};

void main(void)
{
    vec2 scale = in_Scale_world;
    mat4 modelToWorld = mat4( 
        vec4(scale.x, 0.0,     0.0, 0.0),
        vec4(0.0,     scale.y, 0.0, 0.0),
        vec4(0.0,     0.0,     1.0, 0.0),
        in_Position_world
    );

    gl_Position = u_Projection * u_Camera * modelToWorld * ve_Position_model;
    //gl_Position = modelToWorld * ve_Position_model;
    //ex_Color = in_VertexColor;
    ex_Color = vec4(1., 1., 1., 1.);
}
)#";


const std::string gFragmentShader = R"#(
#version 400

in vec4 ex_Color;

out vec4 out_Color;

void main(void)
{
    out_Color = ex_Color;
}
)#";


} // namespace ShapingShaders
} // namespace r2d
} // namespace graphics
} // namespaceadd
