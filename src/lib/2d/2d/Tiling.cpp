#include "Tiling.h"

#include "Engine.h"
#include "Vertex.h"

#include <handy/vector_utils.h>

#include <Math/Range.h>

namespace ad {

const GLchar* gTilingVertexShader = R"#(
    #version 400

    layout(location=0) in vec4  in_VertexPosition;
    layout(location=1) in ivec2 in_UV;
    layout(location=2) in vec2  in_InstancePosition;
    //layout(location=3) in ivec4 in_TextureArea;
    layout(location=3) in vec3 in_Color;

    uniform ivec2 in_BufferResolution;
    
    out vec2 ex_UV;
    out vec3 ex_Color;

    void main(void)
    {
        vec2 bufferSpacePosition = in_InstancePosition + in_VertexPosition.xy;
        gl_Position = vec4(2 * bufferSpacePosition / in_BufferResolution - vec2(1.0, 1.0),
                           0.0, 1.0);

        //ex_UV = in_TextureArea.xy + in_UV*in_TextureArea.zw;
        ex_Color = in_Color;
    }
)#";


const GLchar* gTilingFragmentShader = R"#(
    #version 400

    in vec2 ex_UV;
    in vec3 ex_Color;
    out vec4 out_Color;
    uniform sampler2DRect spriteSampler;

    void main(void)
    {
        //out_Color = texture(spriteSampler, ex_UV);
        out_Color = vec4(ex_Color, 1.0);
    }
)#";

VertexSpecification makeVertexGrid(const Size2<int> aCellSize, const Size2<int> aGridDefinition)
{
    VertexSpecification specification;
    glBindVertexArray(specification.mVertexArray); 

    const Vec2<GLint> cellOffset(aCellSize);
    
    // Per-vertex attributes
    std::vector<Vec2<GLint>> UVs = {
        {0, 0},
        {0, 1},
        {1, 0},
        {1, 1}
    };
    std::vector<Vertex> quad;
    for (auto uv : UVs)
    {
        /// \todo This has to be more elegant, the intermediary value should be replaced 
        ///       at least with a cast
        Vec2<GLint> position = cellOffset.hadamard(uv);
        quad.push_back({{position.x(), position.y()}, uv});
    }

    specification.mVertexBuffers.emplace_back(
        makeLoadedVertexBuffer<Vertex>(gVertexDescription, quad.cbegin(), quad.cend()));
    
    // Per-instance attributes
    std::vector<Position> positions;
    for (int x : math::range(aGridDefinition.width()))
    {
        for (int y : math::range(aGridDefinition.height()))
        {
            Vec2<GLint> position = cellOffset.hadamard(Vec2<GLint>{x, y});
            positions.emplace_back(position.x(), position.y());
        }
    }
    specification.mVertexBuffers.push_back(
        makeLoadedVertexBuffer<Position>({
                {2, 2, 0, MappedGL<GLint>::enumerator}
            },
            positions.cbegin(),
            positions.cend()));

    glVertexAttribDivisor(2, 1);

    /// \todo separate buffer specification and filling
    specification.mVertexBuffers.push_back(
        makeLoadedVertexBuffer({
                {3, 3, 0, MappedGL<GLubyte>::enumerator, ShaderAccess::Float, true}
            },
            0, 
            0,
            nullptr));

    glVertexAttribDivisor(3, 1);

    return specification;
}


Program makeProgram()
{
    Program program = makeLinkedProgram({
                          {GL_VERTEX_SHADER, gTilingVertexShader},
                          {GL_FRAGMENT_SHADER, gTilingFragmentShader},
                      });

    glProgramUniform1i(program, glGetUniformLocation(program, "spriteSampler"), 0);

    return program;
}

Tiling::Tiling(Size2<int> aCellSize, Size2<int> aGridDefinition) :
    mColors(aGridDefinition.area(), Color{0, 0, 0}),
    mDrawContext(makeVertexGrid(aCellSize, aGridDefinition), makeProgram())
{}


// std::vector<LoadedSprite> Tiling::load(const SpriteSheet &aSpriteSheet)
// {}

Tiling::instance_data::iterator Tiling::begin()
{
    return mColors.begin(); 
}

Tiling::instance_data::iterator Tiling::end()
{
    return mColors.end(); 
}

void Tiling::render(const Engine & aEngine) const
{
    activate(mDrawContext);

    GLint location = glGetUniformLocation(mDrawContext.mProgram, "in_BufferResolution");
    glProgramUniform2iv(mDrawContext.mProgram, location, 1, aEngine.getWindowSize().data());

    //
    // Stream vertex attributes
    //
    respecifyBuffer(mDrawContext.mVertexSpecification.mVertexBuffers.back(),
                    mColors.data(),
                    getStoredSize(mColors));

    //
    // Draw
    //
    glDrawArraysInstanced(GL_TRIANGLE_STRIP,
                          0,
                          gVerticesPerInstance,
                          static_cast<GLsizei>(mColors.size()));
}

} // namespace ad
