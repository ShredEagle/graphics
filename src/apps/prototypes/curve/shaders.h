#pragma once


#include <glad/glad.h>


namespace ad {

const GLchar* gLineVertexShader = R"#(
    #version 400

    uniform mat4 projection;
    layout(location=0) in vec2 in_Position;

    void main(void)
    {
        gl_Position = projection * vec4(in_Position, 0.0, 1.0);
    }
)#";


// Note: With this approach, the decomposition is computed twice for each
// t value (once for each `side`).
const GLchar* gGenerativeVertexShader = R"#(
    #version 400

    layout(location=0) in float in_t;
    layout(location=1) in int in_side;

    layout(location=2) in vec2 p0;
    layout(location=3) in vec2 p1;
    layout(location=4) in vec2 p2;
    layout(location=5) in vec2 p3;

    uniform mat4 projection;
    uniform float halfWidth;

    void main(void)
    {
        // Unfolded DeCasteljau for degree 3
        vec2 ab = (1-in_t) * p0 + in_t * p1;
        vec2 bc = (1-in_t) * p1 + in_t * p2;
        vec2 cd = (1-in_t) * p2 + in_t * p3;

        vec2 ac = (1-in_t) * ab + in_t * bc;
        vec2 bd = (1-in_t) * bc + in_t * cd;

        vec2 ad = (1-in_t) * ac + in_t * bd;

        // This approach does not work for the first point
        // where t == 0 => ad == ac == p0 => (ad - ac) is the null vector.
        //vec2 normal = normalize(vec2(ad.y - ac.y, ac.x - ad.x));

        vec2 tangent = ac - bd;
        vec2 normal = normalize(vec2(-tangent.y, tangent.x));

        gl_Position = projection * vec4(
            ad + normal * in_side * halfWidth,
            0.,
            1.);
    }
)#";


const GLchar* gFragmentShader = R"#(
    #version 400

    out vec4 out_Color;

    void main(void)
    {
        out_Color = vec4(1.0, 0.0, 1.0, 1.0);
    }
)#";


} // namespace ad
