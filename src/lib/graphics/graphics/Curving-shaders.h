#pragma once


#include <glad/glad.h>


namespace ad {
namespace graphics {


// IMPORTANT:
// > If an inline function or variable (since C++17) with external linkage is defined differently 
// > in different translation units, the behavior is undefined.
// This made for a very efficient way to lose ton of time chasing down an Heisenbug.
// Add a scoping namespace

namespace curving {

inline const GLchar* gVertexShader = R"#(
    #version 400

    layout(location=0) in float t;
    layout(location=1) in int side;

    layout(location=2) in vec3 p0;
    layout(location=3) in vec3 p1;
    layout(location=4) in vec3 p2;
    layout(location=5) in vec3 p3;

    layout(location=6) in float startHalfWidth;
    layout(location=7) in float endHalfWidth;

    layout(location=8) in mat4 model;

    uniform mat4 u_camera;
    uniform mat4 u_projection;

    void main(void)
    {
        // Unfolded DeCasteljau for degree 3
        vec3 ab = (1 - t) * p0 + t * p1;
        vec3 bc = (1 - t) * p1 + t * p2;
        vec3 cd = (1 - t) * p2 + t * p3;

        vec3 ac = (1 - t) * ab + t * bc;
        vec3 bd = (1 - t) * bc + t * cd;

        vec3 ad = (1 - t) * ac + t * bd;

        //// Works only when points are already in camera space
        //// (because this way the tangent is also in camera space)
        //vec3 tangent = ac - bd;
        //vec3 normal = normalize(vec3(-tangent.y, tangent.x, 0.));
        //gl_Position = u_projection * u_camera * model * vec4(
        //    ad + normal * side * mix(startHalfWidth, endHalfWidth, t),
        //    1.);

        //// Important: does not work when the tangent is alongside the view direction
        //// (we would need to draw a circle, i.e. the normal are all unit vectors in the screen plane)
        //mat4 transform = u_camera * model;
        //vec4 ad_camera = transform * vec4(ad, 1.);
        //vec4 tangent_camera = transform * vec4(ac - bd, 0.);
        //vec4 normal_camera  = normalize(vec4(-tangent_camera.y, tangent_camera.x, 0., 0.));
        //gl_Position = u_projection * (ad_camera + normal_camera * side * mix(startHalfWidth, endHalfWidth, t));


        // Important: does not work when the tangent is alongside the view direction
        // (we would need to draw a circle, i.e. the normal are all unit vectors in the screen plane)
        mat4 transform = u_camera * model;
        vec4 ad_camera = transform * vec4(ad, 1.);
        // Normalizing ensure the tangents is measuring 1 before transformation.
        // This later allows to scale the normal by the transformed tangent length:
        // it accounts for any scaling that might be embedded in modeling or camera matrices.
        vec4 tangent_camera = transform * normalize(vec4(ac - bd, 0.));
        // Important: needs to be normalized again, because Z component is dropped.
        vec4 normal_camera  = normalize(vec4(-tangent_camera.y, tangent_camera.x, 0., 0.));
        gl_Position =
            u_projection * (ad_camera 
                            + normal_camera * side * mix(startHalfWidth, endHalfWidth, t) * length(tangent_camera));

    }
)#";


const GLchar* gFragmentShader = R"#(
    #version 400

    out vec4 out_Color;

    uniform vec3 u_color;

    void main(void)
    {
        out_Color = vec4(u_color, 1.0);
    }
)#";


} // namespace curving
} // namespace graphics
} // namespace ad
