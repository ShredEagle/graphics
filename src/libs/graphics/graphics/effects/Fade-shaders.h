#pragma once


#include <glad/glad.h>


namespace ad {
namespace graphics {
namespace fade {


/// \brief Inteded to samples on texel centers (i.e. to be used with NEAREST filtering)
inline const GLchar* gFragmentShader = R"#(
#version 400

in vec2 ex_TextureUV;
out vec4 out_Color;

uniform sampler2D image;
uniform vec4 fadeToColor;

void main(void)
{
    out_Color = 
        texture(image, ex_TextureUV) * (1 - fadeToColor.a) 
        + fadeToColor;
}

)#";


} // namespace fade
} // namespace graphics
} // namespace ad

