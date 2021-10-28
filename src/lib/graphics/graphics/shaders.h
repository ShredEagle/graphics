#pragma once

namespace ad {
namespace graphics {

inline const GLchar* gVertexShader = R"#(
    #version 400

    layout(location=0) in vec2 in_VertexPosition;
    layout(location=1) in ivec2 in_UV;
    layout(location=2) in vec2 in_InstancePosition;
    layout(location=3) in ivec4 in_TextureArea;
    layout(location=4) in float in_Opacity;

    uniform ivec2 in_BufferResolution;
    
    out vec2  ex_UV;
    out float ex_Opacity;

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

        vec2 bufferSpacePosition = in_InstancePosition + in_VertexPosition * in_TextureArea.zw;
        gl_Position = vec4(2 * bufferSpacePosition / in_BufferResolution - vec2(1.0, 1.0),
                           0.0, 1.0);

        ex_UV = in_TextureArea.xy + in_UV*in_TextureArea.zw;
        ex_Opacity = in_Opacity;
    }
)#";


inline const GLchar* gAnimationFragmentShader = R"#(
    #version 400

    in vec2  ex_UV;
    in float ex_Opacity;
    out vec4 out_Color;
    uniform sampler2DRect spriteSampler;

    void main(void)
    {
        out_Color = texture(spriteSampler, ex_UV) * ex_Opacity;
    }
)#";


//
// Trivial Shaping
//
inline const GLchar* gSolidColorInstanceVertexShader = R"#(
    #version 400

    layout(location=0) in vec4  v_VertexPosition;
    layout(location=1) in vec2  in_InstancePosition;
    layout(location=2) in vec2  in_InstanceDimension;
    layout(location=3) in mat3  in_ModelTransform;
    layout(location=6) in vec3  in_InstanceColor;

    uniform mat3 u_camera;
    uniform mat3 u_projection;

    out vec3 ex_Color;
    
    void main(void)
    {
        vec3 worldPosition = in_ModelTransform 
                             * vec3(v_VertexPosition.xy * in_InstanceDimension + in_InstancePosition, 1.);
        vec3 transformed = u_projection * u_camera * worldPosition;
        gl_Position = vec4(transformed.x, transformed.y, 0., 1.);

        ex_Color = in_InstanceColor;
    }
)#";


//
// TrivialLineStrip
//
inline const GLchar* gTrivialColorVertexShader = R"#(
    #version 400

    layout(location=0) in vec2  in_VertexPosition;
    layout(location=1) in vec3  in_VertexColor;

    out vec3 ex_Color;

    uniform mat3 camera;
    uniform mat3 projection;
    
    void main(void)
    {
        gl_Position = vec4(projection * camera * vec3(in_VertexPosition, 1.), 1.);
        ex_Color = in_VertexColor;
    }
)#";


inline const GLchar* gTrivialFragmentShader = R"#(
    #version 400

    in vec3 ex_Color;
    out vec4 out_Color;

    void main(void)
    {
        out_Color = vec4(ex_Color, 1.);
    }
)#";


//
// Draw Line
//
inline const GLchar* gSolidColorLineVertexShader = R"#(
    #version 400

    layout(location=0) in vec2  in_VertexPosition;
    layout(location=1) in vec2  in_origin;
    layout(location=2) in vec2  in_end;
    layout(location=3) in float in_width;
    layout(location=4) in vec3  in_InstanceColor;

    out vec3 ex_Color;

    uniform ivec2 in_BufferResolution;
    
    out vec2 ex_UV;

    void main(void)
    {
        vec2 direction = in_end - in_origin;
        vec2 orthogonalVec = normalize(vec2(direction.y, -direction.x));
        vec2 bufferSpacePosition = in_origin + in_VertexPosition.y * direction + in_width / 2 * orthogonalVec - in_width * in_VertexPosition.x * orthogonalVec;
        gl_Position = vec4(2 * bufferSpacePosition / in_BufferResolution - vec2(1.0, 1.0),
                           0.0, 1.0);

        ex_Color = in_InstanceColor;
    }
)#";


inline const GLchar* gPassthroughVertexShader = R"#(
#version 400

layout (location=0) in vec4 ve_Position;
layout (location=1) in vec2 ve_TextureUV;

out vec2 ex_TextureUV;

void main(void)
{
    gl_Position = ve_Position;
    ex_TextureUV = ve_TextureUV;
}
)#";


inline const GLchar* gTexturingFragmentShader = R"#(
#version 400

in vec2 ex_TextureUV;
out vec4 out_Color;

uniform sampler2D inputTexture;

void main(void)
{
    out_Color = texture(inputTexture, ex_TextureUV);
}
)#";


} // namespace graphics
} // namespace ad
