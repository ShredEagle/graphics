#pragma once


#include <glad/glad.h>


namespace ad {
namespace graphics {
namespace gaussianblur {


inline const GLchar* gHorizontalBlurFragmentShader = R"#(
#version 400

in vec2 ex_TextureUV;
out vec4 out_Color;

void main(void)
{
    out_Color = vec4(1.0, 0., 0., 1.);
}
)#";


inline const GLchar* gVerticalBlurFragmentShader = R"#(
)#";


} // namespace gaussianblur
} // namespace graphics
} // namespace ad

