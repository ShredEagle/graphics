#version 400

layout(location=0) in vec4 in_Position;
out vec4 ex_Color;

void main(void)
{
    gl_Position = in_Position * vec4(0.8, 0.8, 0.8, 1.0);
    ex_Color = vec4(1.0, 0.5, 0.1, 1.0);
}
