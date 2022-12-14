#include "Introspection.h"

#include <graphics/ApplicationGlfw.h>
#include <renderer/Shading.h>


using namespace ad;


//
// Some basic implementation of Phong illumination model.
// Serving as a candidate for introspection.
//
inline const GLchar* gVertexShader = R"#(
#version 430

layout(location=0) in vec3 ve_VertexPosition_l;
layout(location=1) in vec3 ve_Normal_l;

layout(location=4) in mat4 in_LocalToWorld;
layout(location=8) in mat4 in_LocalToWorldInverseTranspose;
in vec4 in_Albedo; // no explicit location

layout(std140) uniform ViewingBlock
{
    mat4 u_Camera;
    mat4 u_Projection;
};

layout(std140) buffer Gratuitous
{
    ivec2 u_MyVecTwoInt;
    float u_MyFloat;
};

out vec4 ex_Position_v;
out vec4 ex_Normal_v;
out vec3 ex_DiffuseColor;
out vec3 ex_SpecularColor;
out float ex_Opacity;

void main(void)
{
    mat4 localToCamera = u_Camera * in_LocalToWorld;
    ex_Position_v = localToCamera * vec4(ve_VertexPosition_l, 1.f);
    gl_Position = u_Projection * ex_Position_v;
    ex_Normal_v = localToCamera * vec4(ve_Normal_l, 0.f);

    ex_DiffuseColor  = in_Albedo.rgb;
    ex_SpecularColor = in_Albedo.rgb;
    ex_Opacity       = in_Albedo.a;
}
)#";


inline const GLchar* gFragmentShader = R"#(
#version 430

in vec4 ex_Position_v;
in vec4 ex_Normal_v;
in vec3 ex_DiffuseColor;
in vec3  ex_SpecularColor;
in float ex_Opacity;

uniform vec3 u_LightPosition_v;
uniform vec3 u_LightColor;
uniform vec3 u_AmbientColor;

out vec4 out_Color;

void main(void)
{
    // Everything in camera space
    vec3 light = normalize(u_LightPosition_v - ex_Position_v.xyz);
    vec3 view = vec3(0., 0., 1.);
    vec3 h = normalize(view + light);
    vec3 normal = normalize(ex_Normal_v.xyz); // cannot normalize in vertex shader, as interpolation changes length.
    
    float specularExponent = 32;

    vec3 diffuse = 
        ex_DiffuseColor * (u_AmbientColor + u_LightColor * max(0.f, dot(normal, light)));
    vec3 specular = 
        u_LightColor * ex_SpecularColor * pow(max(0.f, dot(normal, h)), specularExponent);
    vec3 color = diffuse + specular;

    // Gamma correction
    float gamma = 2.2;
    out_Color = vec4(pow(color, vec3(1./gamma)), ex_Opacity);
}
)#";



int main(void)
{
    // Solely to get an OpenGL context
    graphics::ApplicationGlfw app{"dummy", {1, 1}};

    graphics::Program program{graphics::makeLinkedProgram({
        {GL_VERTEX_SHADER,   {gVertexShader, "PhongVertexShader"}},
        {GL_FRAGMENT_SHADER, {gFragmentShader, "PhongFragmentShader"}},
    })};

    inspectProgram(program);

    std::exit(EXIT_SUCCESS);
}