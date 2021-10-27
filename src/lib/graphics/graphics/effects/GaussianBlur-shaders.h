#pragma once


#include <glad/glad.h>


namespace ad {
namespace graphics {
namespace gaussianblur {


inline const GLchar* gHorizontalBlurFragmentShader = R"#(
#version 400

in vec2 ex_TextureUV;
out vec4 out_Color;

uniform float weight[5] = float[](
    0.2270270270,
    0.1945945946,
    0.1216216216,
    0.0540540541,
    0.0162162162);

uniform sampler2D image;
vec2 offset = vec2(1.0, 0.0) / textureSize(image, 0);

void main(void)
{
    out_Color = texture(image, ex_TextureUV) * weight[0];
    for (int i = 1; i != weight.length(); ++i)
    {
        out_Color += texture(image, ex_TextureUV + i * offset) * weight[i];
        out_Color += texture(image, ex_TextureUV - i * offset) * weight[i];
    }
}
)#";


inline const GLchar* gVerticalBlurFragmentShader = R"#(
#version 400

in vec2 ex_TextureUV;
out vec4 out_Color;

uniform float weight[5] = float[](
    0.2270270270,
    0.1945945946,
    0.1216216216,
    0.0540540541,
    0.0162162162);

uniform sampler2D image;
vec2 offset = vec2(0.0, 1.0) / textureSize(image, 0);

void main(void)
{
    out_Color = texture(image, ex_TextureUV) * weight[0];
    for (int i = 1; i != weight.length(); ++i)
    {
        out_Color += texture(image, ex_TextureUV + i * offset) * weight[i];
        out_Color += texture(image, ex_TextureUV - i * offset) * weight[i];
    }
}
)#";


} // namespace gaussianblur
} // namespace graphics
} // namespace ad

