#pragma once


#include <glad/glad.h>


namespace ad {
namespace graphics {
namespace gaussianblur {


/// \brief Inteded to samples on texel centers (i.e. to be used with NEAREST filtering)
inline const GLchar* g1DFilterFragmentShader = R"#(
#version 400

in vec2 ex_TextureUV;
out vec4 out_Color;

uniform sampler2D image;
uniform float weights[5];
uniform vec2 filterDirection;

// It seems the normalized texture coordinates are starting on the texture borders
// see: https://stackoverflow.com/a/37484800/1027706
// So it should not be necessary to subtract (1, 1) from the size to get the correct offset.
vec2 offset = filterDirection / textureSize(image, 0) /*- vec2(1., 1.)*/;

void main(void)
{
    out_Color = texture(image, ex_TextureUV) * weights[0];
    for (int i = 1; i != weights.length(); ++i)
    {
        out_Color += texture(image, ex_TextureUV + i * offset) * weights[i];
        out_Color += texture(image, ex_TextureUV - i * offset) * weights[i];
    }
}
)#";


inline const GLchar* g1DLinearFilterFragmentShader = R"#(
#version 400

in vec2 ex_TextureUV;
out vec4 out_Color;

uniform sampler2D image;
uniform float offsets[3];
uniform float weights[3];
uniform vec2 filterDirection;

vec2 offsets_texturespace[3] = vec2[](
    filterDirection * offsets[0] / textureSize(image, 0),
    filterDirection * offsets[1] / textureSize(image, 0),
    filterDirection * offsets[2] / textureSize(image, 0));

void main(void)
{
    out_Color = texture(image, ex_TextureUV) * weights[0];
    for (int i = 1; i != weights.length(); ++i)
    {
        out_Color += texture(image, ex_TextureUV + offsets_texturespace[i]) * weights[i];
        out_Color += texture(image, ex_TextureUV - offsets_texturespace[i]) * weights[i];
    }
}
)#";

} // namespace gaussianblur
} // namespace graphics
} // namespace ad

