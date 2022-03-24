#pragma once


namespace ad {
namespace gltfviewer {


//
// Naive (ambient only)
//
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

    uniform vec4 u_baseColorFactor;

    out vec4 out_color;

    void main(void)
    {
        out_color = u_baseColorFactor;
    }
)#";


//
// Phong shading(ambient, diffuse, specular)
//
inline const GLchar* gPhongVertexShader = R"#(
    #version 400

    layout(location=0) in vec4 ve_position;
    layout(location=1) in vec3 ve_normal;
    layout(location=2) in vec2 ve_baseColorUv;

    layout(location=8) in mat4 in_modelTransform;

    uniform mat4 u_camera;
    uniform mat4 u_projection;

    out vec4 ex_position_view;
    out vec4 ex_normal_view;
    out vec2 ex_baseColorUv;

    void main(void)
    {
        mat4 modelViewTransform = u_camera * in_modelTransform;
        ex_position_view = modelViewTransform * ve_position;
        ex_normal_view = vec4(
            normalize(transpose(inverse(mat3(modelViewTransform))) * ve_normal),
            0.);
        ex_baseColorUv = ve_baseColorUv;

        gl_Position = u_projection * ex_position_view;
    }
)#";


inline const GLchar* gPhongFragmentShader = R"#(
    #version 400

    struct Light
    {
        vec4 position_world;
        vec3 color;
        int specularExponent;
        float ambient;
    };

    in vec4 ex_position_view;
    in vec4 ex_normal_view;
    in vec2 ex_baseColorUv;

    uniform vec4 u_baseColorFactor;
    uniform Light u_light;
    uniform mat4 u_camera;
    uniform sampler2D u_baseColorTex;

    out vec4 out_color;

    vec3 sRgbToLinear(vec3 sRgb)
    {
        //see: http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
        return sRgb * (sRgb * (sRgb * 0.305306011 + 0.682171111) + 0.012522878);
    }

    void main(void)
    {
        vec4 materialColor = u_baseColorFactor * texture(u_baseColorTex, ex_baseColorUv);

        vec4 lightDirection_view = normalize(u_camera * u_light.position_world - ex_position_view);
        vec4 bisector_view = vec4(normalize(vec3(0., 0., 1.) + lightDirection_view.xyz), 0.);

        // Use the same color for all lighting components (diffuse, specular and ambient).
        out_color = vec4(
            materialColor.xyz * u_light.color
                * ( max(0., dot(ex_normal_view, lightDirection_view)) // diffuse
                    + max(0., pow(dot(ex_normal_view, bisector_view), u_light.specularExponent)) // specular
                    + u_light.ambient // ambient
                   )
            ,
            materialColor.w);
    }
)#";


} // namespace gltfviewer
} // namespace ad
