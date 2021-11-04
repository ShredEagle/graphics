#pragma once


#include <glad/glad.h>


namespace ad {
namespace font {


const GLchar* gFontVertexShader = R"#(
    #version 400
    
    layout(location=0) in vec2 ve_Position_u;
    // TODO should it be integral?
    layout(location=1) in vec2 ve_UV;

    layout(location=2) in vec2  in_Position_w;
    layout(location=3) in ivec2 in_TextureOffset; // implicit 0 on y
    layout(location=4) in vec2  in_BoundingBox;
    layout(location=5) in vec2  in_Bearing;
    
    uniform vec2 u_PixelToWorld;
    uniform mat4 u_WorldToCamera;
    uniform mat4 u_Projection;

    out vec2  ex_TextureUV;

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

        vec2 worldBearing     = in_Bearing * u_PixelToWorld;
        vec2 worldBoundingBox = in_BoundingBox * u_PixelToWorld;
        vec2 worldPosition = in_Position_w + worldBearing + (ve_Position_u * worldBoundingBox);

        gl_Position = vec4(worldPosition, 0., 1.) * u_WorldToCamera * u_Projection;
        ex_TextureUV = in_TextureOffset + (ve_UV * in_BoundingBox);
    }
    
)#";



const GLchar* gFontFragmentShader = R"#(
    #version 400

    in vec2 ex_TextureUV;
    out vec4 out_Color;

    uniform sampler2DRect inputTexture;

    void main(void)
    {
        float alpha = texture(inputTexture, ex_TextureUV).r;
        out_Color = vec4(1., 1., 1., alpha);
    }
)#";


} // namespace font
} // namespace ad
