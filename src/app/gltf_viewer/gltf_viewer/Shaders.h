#pragma once


namespace ad {
namespace gltfviewer {


//
// Phong shading(ambient, diffuse, specular)
//
inline const GLchar* gPhongVertexShader = R"#(
    #version 400

    layout(location=0) in vec4 ve_position;
    layout(location=1) in vec3 ve_normal;
    layout(location=2) in vec2 ve_baseColorUv;
    layout(location=3) in vec4 ve_color;

    layout(location=8) in mat4 in_modelTransform;

    uniform mat4 u_camera;
    uniform mat4 u_projection;
    uniform vec4 u_vertexColorOffset;

    out vec4 ex_position_view;
    out vec4 ex_normal_view;
    out vec2 ex_baseColorUv;
    out vec4 ex_color;

    void main(void)
    {
        mat4 modelViewTransform = u_camera * in_modelTransform;
        ex_position_view = modelViewTransform * ve_position;
        ex_normal_view = vec4(
            normalize(transpose(inverse(mat3(modelViewTransform))) * ve_normal),
            0.);
        ex_baseColorUv = ve_baseColorUv;
        ex_color = ve_color + u_vertexColorOffset;

        gl_Position = u_projection * ex_position_view;
    }
)#";


inline const GLchar* gSkeletalVertexShader = R"#(
    #version 400

    layout(location=0) in vec4 ve_position;
    layout(location=1) in vec3 ve_normal;
    layout(location=2) in vec2 ve_baseColorUv;
    layout(location=3) in vec4 ve_color;
    layout(location=4) in vec4 ve_joints; //TODO ivec4
    layout(location=5) in vec4 ve_weights;

    // The client submit skinned geomatry via non-instanced draw calls,
    // so the instance buffer is empty (attempting to access does crash).
    // Also, the whole model to world transformation must be in the palette:
    // > Only the joint transforms are applied to the skinned mesh;
    // > the transform of the skinned mesh node MUST be ignored.
    //layout(location=8) in mat4 in_modelTransform;

    uniform mat4 u_camera;
    uniform mat4 u_projection;
    uniform vec4 u_vertexColorOffset;

    layout(std140) uniform MatrixPalette
    {
        mat4 joints[64];
    };

    out vec4 ex_position_view;
    out vec4 ex_normal_view;
    out vec2 ex_baseColorUv;
    out vec4 ex_color;

    void main(void)
    {
        mat4 skinningMatrix = 
              joints[int(ve_joints.x)] * ve_weights.x
            + joints[int(ve_joints.y)] * ve_weights.y
            + joints[int(ve_joints.z)] * ve_weights.z
            + joints[int(ve_joints.w)] * ve_weights.w;

        mat4 modelViewTransform = u_camera * skinningMatrix;
        ex_position_view = modelViewTransform * ve_position;
        ex_normal_view = vec4(
            normalize(transpose(inverse(mat3(modelViewTransform))) * ve_normal),
            0.);
        ex_baseColorUv = ve_baseColorUv;
        ex_color = ve_color + u_vertexColorOffset;

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
        float diffuse;
        float specular;
        float ambient;
    };

    in vec4 ex_position_view;
    in vec4 ex_normal_view;
    in vec2 ex_baseColorUv;
    in vec4 ex_color;

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
        vec4 materialColor = 
            u_baseColorFactor * texture(u_baseColorTex, ex_baseColorUv)
            * ex_color
            ;

        vec4 lightDirection_view = normalize(u_camera * u_light.position_world - ex_position_view);
        vec4 bisector_view = vec4(normalize(vec3(0., 0., 1.) + lightDirection_view.xyz), 0.);

        // Use the same color for all lighting components (diffuse, specular and ambient).
        out_color = vec4(
            materialColor.xyz * u_light.color
                * ( max(0., dot(ex_normal_view, lightDirection_view)) * u_light.diffuse  // diffuse
                    + max(0., pow(dot(ex_normal_view, bisector_view), u_light.specularExponent)) * u_light.ambient // specular
                    + u_light.ambient // ambient
                   )
            ,
            materialColor.w);
            //1.);

        //out_color = ex_normal_view;
        //out_color = materialColor;
    }
)#";


} // namespace gltfviewer
} // namespace ad
