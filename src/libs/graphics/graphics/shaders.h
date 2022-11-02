#pragma once

namespace ad {
namespace graphics {


inline const GLchar* gSpriteVertexShader = R"#(
    #version 400

    layout(location=0) in vec2  ve_VertexPosition;
    layout(location=1) in ivec2 ve_UV;

    layout(location=2) in mat3  in_ModelTransform;
    layout(location=5) in ivec4 in_TextureArea;
    layout(location=6) in float in_Opacity;
    layout(location=7) in vec2  in_AxisMirroring;

    uniform vec2 u_pixelWorldSize;
    uniform mat3 u_camera;
    uniform mat3 u_projection;

    out vec2  ex_UV;
    out float ex_Opacity;

    void main(void)
    {
        vec3 vertexPosition_local = vec3(ve_VertexPosition * in_TextureArea.zw * u_pixelWorldSize, 1.);
        vec3 vertexPosition_world = in_ModelTransform * vertexPosition_local;
        vec3 vertexPosition_ndc = u_projection * u_camera * vertexPosition_world;

        gl_Position = vec4(vertexPosition_ndc.x, vertexPosition_ndc.y, 0., 1.);

        // Handle sprite mirroring
        vec2 uv = ve_UV - vec2(0.5, 0.5);
        uv *= in_AxisMirroring;
        uv += vec2(0.5, 0.5);

        ex_UV = in_TextureArea.xy + uv * in_TextureArea.zw;
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

    out vec4 ex_Color;

    uniform mat3 camera;
    uniform mat3 projection;
    
    void main(void)
    {
        gl_Position = vec4(projection * camera * vec3(in_VertexPosition, 1.), 1.);
        ex_Color = vec4(in_VertexColor, 1.);
    }
)#";


inline const GLchar* gTrivialFragmentShader = R"#(
    #version 400

    in vec4 ex_Color;
    out vec4 out_Color;

    void main(void)
    {
        out_Color = ex_Color;
    }
)#";


// TODO Ad 2022/04/01: Replace the non-opacity version with this one
inline const GLchar* gTrivialFragmentShaderOpacity = R"#(
    #version 400

    in vec4 ex_Color;
    out vec4 out_Color;

    void main(void)
    {
        out_Color = ex_Color;
    }
)#";

//
// TrivialPolygon
// 
inline const GLchar* gTrivialColorTransformVertexShader = R"#(
    #version 400

    layout(location=0) in vec2  in_VertexPosition;
    layout(location=1) in vec3  in_VertexColor;
    layout(location=3) in mat3  in_ModelTransform;

    out vec3 ex_Color;

    uniform mat3 camera;
    uniform mat3 projection;
    
    void main(void)
    {
        gl_Position = vec4(projection * camera * in_ModelTransform * vec3(in_VertexPosition, 1.), 1.);
        ex_Color = in_VertexColor;
    }
)#";


namespace drawline {

    inline const GLchar* gSolidColorLineVertexShader = R"#(
        #version 400

        layout(location=0) in vec2  ve_position;

        layout(location=1) in vec4  in_origin;
        layout(location=2) in vec4  in_end;
        layout(location=3) in float in_width_screen;
        layout(location=4) in vec4  in_color;

        uniform mat4 u_view;
        uniform mat4 u_projection;
        uniform ivec2 in_BufferResolution;

        out vec4 ex_Color;
        
        void main(void)
        {
            vec4 origin_view = u_view * in_origin;
            vec4 direction_view = u_view * (in_end - in_origin);

            // Project the normal on view plane by discarding Z, and rotate by 90 degree CCW.
            vec4 normal_view = vec4(-direction_view.y, direction_view.x, 0., 0.);
            // We need the normal (normalized) in clip space, because the width is given in screen space.
            vec2 normal_clip = normalize(vec2(u_projection * normal_view));
            vec4 halfWidth_clip = vec4((in_width_screen * normal_clip / in_BufferResolution), 0., 0.);

            gl_Position = u_projection * (origin_view + ve_position.x * direction_view)
                          + ve_position.y * halfWidth_clip;

            ex_Color = in_color;
        }
    )#";

} // namespace drawline

inline const GLchar* gPassthroughVertexShader = R"#(
#version 400

layout (location=0) in vec4 ve_Position;
layout (location=1) in vec2 ve_TextureUV;

uniform vec2 u_UVScaling = vec2(1., 1.);

out vec2 ex_TextureUV;

void main(void)
{
    gl_Position = ve_Position;
    ex_TextureUV = ve_TextureUV * u_UVScaling;
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


namespace texting
{
    inline const GLchar* gGlyphVertexShader = R"#(
        #version 400
        
        layout(location=0) in vec2 ve_Position_u;
        layout(location=1) in vec2 ve_UV; // not integral, it is multiplied by the float bbox anyway.

        layout(location=2) in vec2  in_Position_w;
        layout(location=3) in ivec2 in_TextureOffset; // implicit 0 on y
        layout(location=4) in vec2  in_BoundingBox;
        layout(location=5) in vec2  in_Bearing;
        layout(location=6) in vec4  in_Color;
        
        uniform vec2 u_BoundingOffsets_pixel;
        uniform vec2 u_PixelToWorld;
        uniform mat3 u_WorldToCamera;
        uniform mat3 u_Projection;

        out vec2  ex_TextureUV;
        out vec4  ex_Color;

        void main(void)
        {
            // Go back (left) by horizontal offset, but advance (up) by vertical offset.
            vec2 worldBearing     = vec2(in_Bearing.x - u_BoundingOffsets_pixel.x, in_Bearing.y + u_BoundingOffsets_pixel.y) * u_PixelToWorld;
            vec2 worldBoundingBox = in_BoundingBox * u_PixelToWorld;
            vec2 worldPosition    = in_Position_w + worldBearing + (ve_Position_u * worldBoundingBox);

            vec3 transformed = u_Projection * u_WorldToCamera * vec3(worldPosition, 1.);
            gl_Position = vec4(transformed.xy, 0., 1.);
            ex_TextureUV = in_TextureOffset + (ve_UV * in_BoundingBox);
            ex_Color = in_Color;
        }
    )#";

    inline const GLchar* gGlyphFragmentShader = R"#(
        #version 400

        in vec2 ex_TextureUV;
        in vec4 ex_Color;
        out vec4 out_Color;

        uniform sampler2DRect u_FontAtlas;

        void main(void)
        {
            float alpha = texture(u_FontAtlas, ex_TextureUV).r;
            out_Color = ex_Color * vec4(1., 1., 1., alpha);
        }
    )#";
};


} // namespace graphics
} // namespace ad
