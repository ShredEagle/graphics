#pragma once


#include <glad/glad.h>


namespace ad {
namespace graphics {


const GLchar* gVertexShader = R"#(
    #version 400

    layout(location=0) in vec2 in_Position;

    void main(void)
    {
        gl_Position = vec4(in_Position, 0.0, 1.0);
    }
)#";


// Note: No handling of joints, basically creating a concave "dent" on the convex side of the joint.
const GLchar* gNaiveGeometryShader = R"#(
    #version 400
    #extension GL_EXT_geometry_shader4 : enable

    layout(lines) in;
    layout(triangle_strip, max_vertices = 4) out;

    uniform float lineHalfWidth;

    uniform mat4 model;
    uniform mat4 projection;

    void main()
    {
        mat4 transformation = projection * model;

        vec4 normal = vec4(normalize(vec2(gl_PositionIn[0].y - gl_PositionIn[1].y,
                                          gl_PositionIn[1].x - gl_PositionIn[0].x)),
                           0.,
                           0.);

        gl_Position = transformation * (gl_PositionIn[0] + normal * lineHalfWidth);
        EmitVertex();
        gl_Position = transformation * (gl_PositionIn[0] - normal * lineHalfWidth);
        EmitVertex();
        gl_Position = transformation * (gl_PositionIn[1] + normal * lineHalfWidth);
        EmitVertex();
        gl_Position = transformation * (gl_PositionIn[1] - normal * lineHalfWidth);
        EmitVertex();
    }

)#";


// Note: A major drawback with this approach is that it computes twice
// each miter (once as the right miter, then on next invocation as the left miter).
const GLchar* gMiterGeometryShader = R"#(
    #version 400
    #extension GL_EXT_geometry_shader4 : enable

    layout(lines_adjacency) in;
    layout(triangle_strip, max_vertices = 4) out;

    uniform float lineHalfWidth;

    uniform mat4 model;
    uniform mat4 projection;

    void main()
    {
        mat4 transformation = projection * model;

        vec4 left  = gl_PositionIn[1] - gl_PositionIn[0];
        vec4 mid   = gl_PositionIn[2] - gl_PositionIn[1];
        vec4 right = gl_PositionIn[3] - gl_PositionIn[2];

        vec4 leftTangent = normalize(normalize(left) + normalize(mid));
        vec4 rightTangent = normalize(normalize(mid) + normalize(right));

        vec4 leftMiter  = vec4(-leftTangent.y, leftTangent.x, 0., 0.);
        vec4 rightMiter = vec4(-rightTangent.y, rightTangent.x, 0., 0.);

        vec4 normal = vec4(normalize(vec2(gl_PositionIn[1].y - gl_PositionIn[2].y,
                                          gl_PositionIn[2].x - gl_PositionIn[1].x)),
                           0.,
                           0.);

        float leftLength  = lineHalfWidth / dot(normal, leftMiter);
        float rightLength = lineHalfWidth / dot(normal, rightMiter);

        gl_Position = transformation * (gl_PositionIn[1] + leftMiter * leftLength);
        EmitVertex();
        gl_Position = transformation * (gl_PositionIn[1] - leftMiter * leftLength);
        EmitVertex();
        gl_Position = transformation * (gl_PositionIn[2] + rightMiter * rightLength);
        EmitVertex();
        gl_Position = transformation * (gl_PositionIn[2] - rightMiter * rightLength);
        EmitVertex();
    }

)#";


const GLchar* gFragmentShader = R"#(
    #version 400

    out vec4 out_Color;

    void main(void)
    {
        out_Color = vec4(1.0, 1.0, 0.0, 1.0);
    }
)#";


} // namespace graphics
} // namespace ad
