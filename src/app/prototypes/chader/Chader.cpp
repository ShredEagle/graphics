#include "Chader.h"

#include <fstream>
#include <sstream>

namespace ad {
namespace graphics {

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

constexpr std::initializer_list<AttributeDescription> gVertexDescription = {
    { 0, 2, offsetof(VertexChad, mPosition), MappedGL<GLfloat>::enumerator},
};


Chader::Chader() :
    mVertexData(mDrawer.addVertexBuffer(gVertexDescription,
                                        std::span<VertexChad>{gVerticesChad}))
{}

std::string readFile(const boost::filesystem::path & aPath)
{
    std::stringstream buffer;
    buffer << std::ifstream{aPath.string()}.rdbuf();
    return buffer.str();
}

Program makeProgram(const boost::filesystem::path & aVertexShader, const boost::filesystem::path & aFragmentShader)
{
    return makeLinkedProgram({
      {GL_VERTEX_SHADER, readFile(aVertexShader).c_str()},
      {GL_FRAGMENT_SHADER, readFile(aFragmentShader).c_str()},
    });
}


void Chader::loadProgram(const boost::filesystem::path & aVertexShader, const boost::filesystem::path & aFragmentShader)
{
    try
    {
        mDrawer.mProgram = makeProgram(aVertexShader, aFragmentShader);
        std::cout << "Successfully loaded program" << std::endl;
    }
    catch(const ShaderCompilationError & e)
    {
        std::cerr << "Error in the provied shaders: " << e.getErrorLog() << std::endl;
    }
}

void Chader::render() const
{
    mDrawer.render();
}

} // namespace graphics
} // namespace ad
