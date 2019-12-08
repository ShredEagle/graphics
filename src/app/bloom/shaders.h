#pragma once

namespace ad {

static const char* gInitialVertex = R"#(
#version 400

layout(location=0) in vec4 in_Position;
out vec4 ex_Color;

void main(void)
{
    gl_Position = in_Position * vec4(0.8, 0.8, 0.8, 1.0);
    ex_Color = vec4(0.5, 0.25, 0.05, 1.0);
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
layout(location=1) in vec2 in_TexCoords;
out vec2 ex_TexCoords;

void main(void)
{
    gl_Position = in_Position;
    ex_TexCoords = in_TexCoords;
}
)#";

static const char* gScreenFragment = R"#(
#version 400

in vec2 ex_TexCoords;
out vec4 out_Color;

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;

void main(void)
{
    out_Color = texture(sceneTexture, ex_TexCoords)
                + texture(bloomTexture, ex_TexCoords);
}
)#";

static const char* gHBlurFragment = R"#(
#version 400

in vec2 ex_TexCoords;
out vec4 out_Color;

uniform sampler2D screenTexture;
vec2 textureSize = textureSize(screenTexture, 0);

float weights[4] = float[](0.3, 0.2, 0.1, 0.05);

void main(void)
{
    out_Color = weights[0] * texture(screenTexture, ex_TexCoords);
    vec2 texOffset = vec2(1.0/textureSize.x, 0.);

    for(int i=1; i<weights.length(); ++i)
    {
        out_Color += weights[i] * texture(screenTexture, ex_TexCoords - (i*texOffset));
        out_Color += weights[i] * texture(screenTexture, ex_TexCoords + (i*texOffset));
    }
}
)#";

static const char* gVBlurFragment = R"#(
#version 400

in vec2 ex_TexCoords;
out vec4 out_Color;

uniform sampler2D screenTexture;
vec2 textureSize = textureSize(screenTexture, 0);

float weights[4] = float[](0.3, 0.2, 0.1, 0.05);

void main(void)
{
    out_Color = weights[0] * texture(screenTexture, ex_TexCoords);
    vec2 texOffset = vec2(0., 1.0/textureSize.x);

    for(int i=1; i<weights.length(); ++i)
    {
        out_Color += weights[i] * texture(screenTexture, ex_TexCoords - (i*texOffset));
        out_Color += weights[i] * texture(screenTexture, ex_TexCoords + (i*texOffset));
    }
}
)#";

} // namespace ad
