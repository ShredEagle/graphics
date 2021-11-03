#pragma once


#include <glad/glad.h>


namespace ad {
namespace font {


const GLchar* gFontVertexShader = R"#(
    #version 400
)#";



const GLchar* gFontFragmentShader = R"#(
    #version 400

    in vec2 ex_TextureUV;
    out vec4 out_Color;

    uniform sampler2D inputTexture;

    void main(void)
    {
        float alpha = texture(inputTexture, ex_TextureUV).r;
        out_Color = vec4(1., 1., 1., alpha);
    }
)#";


} // namespace font
} // namespace ad
