#pragma once

namespace ad {

static const char* gInitialVertex = R"#(
#version 400

layout(location=0) in vec4 in_Position;
out vec4 ex_Color;

void main(void)
{
    gl_Position = in_Position * vec4(0.8, 0.8, 0.8, 1.0);
    ex_Color = vec4(1.0, 0.5, 0.1, 1.0);
}
)#";

static const char* gInitialFragment = R"#(
#version 400

in vec4 ex_Color;
out vec4 out_Color;

void main(void)
{
    out_Color = ex_Color;
}
)#";

static const char* gScreenVertex = R"#(
#version 400

layout(location=0) in vec4 in_Position;
//layout(location=1) in vec2 in_TexCoords;
out vec2 ex_TexCoords;

void main(void)
{
    gl_Position = in_Position;
    ex_TexCoords = vec2(in_Position.x, in_Position.y)*vec2(800, 600);
}
)#";

static const char* gScreenFragment = R"#(
#version 400

in vec2 ex_TexCoords;
out vec4 out_Color;

uniform sampler2DRect screenTexture;

void main(void)
{
    out_Color = texture(screenTexture, ex_TexCoords);
}
)#";

} // namespace ad
