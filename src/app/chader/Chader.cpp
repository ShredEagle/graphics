#include "Chader.h"

#include <renderer/commons.h>

#include <fstream>
#include <sstream>

namespace ad {

struct VertexChad
{
    Vec2<GLfloat> mPosition;
};

std::array<VertexChad, 4> gVerticesChad = {
    VertexChad{
        {-1.0f, -1.0f},
    },
    VertexChad{
        {-1.0f,  1.0f},
    },
    VertexChad{
        { 1.0f, -1.0f},
    },
    VertexChad{
        { 1.0f,  1.0f},
    },
};

std::string readFile(const path & aPath)
{
    std::stringstream buffer;
    buffer << std::ifstream{aPath}.rdbuf();
    return buffer.str();
}

Program makeProgram(const path & aVertexShader, const path & aFragmentShader)
{
    return makeLinkedProgram({
      {GL_VERTEX_SHADER, readFile(aVertexShader).c_str()},
      {GL_FRAGMENT_SHADER, readFile(aFragmentShader).c_str()},
    });
}


void Chader::loadProgram(const std::filesystem::path & aVertexShader, const std::filesystem::path & aFragmentShader)
{
    mDrawer.mProgram = makeProgram(aVertexShader, aFragmentShader);
}

} // namespace ad
