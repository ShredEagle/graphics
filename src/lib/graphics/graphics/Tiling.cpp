#include "Tiling.h"

#include "AppInterface.h"
#include "Vertex.h"

#include <handy/vector_utils.h>

#include <math/Range.h>

namespace ad {
namespace graphics {


const GLchar* gTilingVertexShader = R"#(
    #version 400

    layout(location=0) in vec4  in_VertexPosition;
    layout(location=1) in ivec2 in_UV;
    layout(location=2) in vec2  in_InstancePosition;
    layout(location=3) in ivec4 in_TextureArea;

    uniform ivec2 in_BufferResolution;
    uniform ivec2 in_GridPosition;

    out vec2 ex_UV;

    void main(void)
    {
        vec2 bufferSpacePosition = in_InstancePosition + in_VertexPosition.xy + in_GridPosition;
        gl_Position = vec4(2 * bufferSpacePosition / in_BufferResolution - vec2(1.0, 1.0),
                           0.0, 1.0);

        ex_UV = in_TextureArea.xy + in_UV*in_TextureArea.zw;
    }
)#";


const GLchar* gTilingFragmentShader = R"#(
    #version 400

    in vec2 ex_UV;
    out vec4 out_Color;
    uniform sampler2DRect spriteSampler;

    void main(void)
    {
        out_Color = texture(spriteSampler, ex_UV);
    }
)#";


// Make a quad of given size, with [0, 1] UV coordinates on Vertices
std::vector<Vertex> makeQuad(const Size2<int> aQuadSize)
{
    std::vector<Vec2<GLint>> UVs = {
        {0, 0},
        {0, 1},
        {1, 0},
        {1, 1}
    };

    std::vector<Vertex> quad;
    for (auto uv : UVs)
    {
        quad.push_back({static_cast<Vec2<GLfloat>>(aQuadSize.as<math::Vec>().cwMul(uv)), uv});
    }
    return quad;
}

std::vector<Position2<GLint>> makePositions(const Vec2<int> aCellOffset,
                                            const Size2<int> aGridDefinition)
{
    std::vector<Position2<GLint>> positions;
    for (int x : math::range(aGridDefinition.width()))
    {
        for (int y : math::range(aGridDefinition.height()))
        {
            Vec2<GLint> position = aCellOffset.cwMul(Vec2<GLint>{x, y});
            positions.emplace_back(position.x(), position.y());
        }
    }
    return positions;
}


VertexSpecification makeVertexGrid(const Size2<int> aCellSize, const Size2<int> aGridDefinition)
{
    VertexSpecification specification;

    // Per-vertex attributes
    std::vector<Vertex> quad = makeQuad(aCellSize);
    specification.mVertexBuffers.emplace_back(loadVertexBuffer(specification.mVertexArray,
                                                               gVertexDescription,
                                                               gsl::make_span(quad)));
    // Could also be
    //makeLoadedVertexBuffer(gVertexDescription, range(quad)));

    //
    // Per-instance attributes
    //

    // The tile position
    const Vec2<GLint> cellOffset(aCellSize);
    std::vector<Position2<GLint>> positions = makePositions(cellOffset, aGridDefinition);
    specification.mVertexBuffers.push_back(
        loadVertexBuffer(specification.mVertexArray,
                         { {2, 2, 0, MappedGL<GLint>::enumerator} },
                         gsl::make_span(positions)));

    glVertexAttribDivisor(2, 1);

    // The tile sprite (as a LoadedSprite, i.e. the rectangle cutout in the image)
    /// \todo separate buffer specification and filling
    specification.mVertexBuffers.push_back(
        loadVertexBuffer(specification.mVertexArray,
                         {
                         
                             //{3, 3, 0, MappedGL<Gubyt>::enumerator, ShaderAccess::Float, true}
                             { {3, Attribute::Access::Integer}, 4, 0, MappedGL<GLint>::enumerator}
                         },
                         sizeof(Tiling::tile_type),
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

    // Matches GL_TEXTURE1 from Tiling::load()
    glProgramUniform1i(program, glGetUniformLocation(program, "spriteSampler"), Tiling::gTextureUnit);

    return program;
}


Tiling::Tiling(Size2<int> aCellSize, Size2<int> aGridDefinition, Size2<int> aRenderResolution) :
    mDrawContext(makeVertexGrid(aCellSize, aGridDefinition), makeProgram()),
    mTiles(aGridDefinition.area(), LoadedSprite{{0, 0}, {0, 0}}),
    mTileSize(aCellSize),
    mGridDefinition(aGridDefinition),
    mGridRectangleScreen{{0.f, 0.f},
                          static_cast<Size2<Tiling::position_t>>(aCellSize.cwMul(aGridDefinition))}
{
    setBufferResolution(aRenderResolution);
}


void Tiling::resetTiling(Size2<int> aCellSize, Size2<int> aGridDefinition)
{
    bindVertexArray(mDrawContext);

    std::vector<Vertex> quad = makeQuad(aCellSize);
    respecifyBuffer(buffers(mDrawContext).at(0), gsl::make_span(quad));

    Vec2<int> cellOffset(aCellSize);
    std::vector<Position2<GLint>> positions = makePositions(cellOffset, aGridDefinition);
    respecifyBuffer(buffers(mDrawContext).at(1), gsl::make_span(positions));

    mTiles.resize(aGridDefinition.area(), LoadedSprite{{0, 0}, {0, 0}});
    mTileSize = aCellSize;
    mGridDefinition = aGridDefinition;
    mGridRectangleScreen.mDimension
        = static_cast<Size2<Tiling::position_t>>(aCellSize.cwMul(aGridDefinition));
}


Tiling::iterator Tiling::begin()
{
    return mTiles.begin();
}


Tiling::iterator Tiling::end()
{
    return mTiles.end();
}


void Tiling::setBufferResolution(Size2<int> aNewResolution)
{
    GLint location = glGetUniformLocation(mDrawContext.mProgram, "in_BufferResolution");
    glProgramUniform2iv(mDrawContext.mProgram, location, 1, aNewResolution.data());
}


void Tiling::setPosition(Position2<position_t> aPosition)
{
    mGridRectangleScreen.mPosition = aPosition;
    GLint location = glGetUniformLocation(mDrawContext.mProgram, "in_GridPosition");
    glProgramUniform2iv(mDrawContext.mProgram, location, 1,
                        static_cast<Position2<GLint>>(mGridRectangleScreen.mPosition).data());
}

void Tiling::render(const AppInterface & aAppInterface) const
{
    activate(mDrawContext);

    //
    // Stream vertex attributes
    //

    // The last buffer, i.e. the sprite corresponding to each tile
    respecifyBuffer(mDrawContext.mVertexSpecification.mVertexBuffers.back(),
                    mTiles.data(),
                    static_cast<GLsizei>(getStoredSize(mTiles)));

    //
    // Draw
    //
    bind_guard scopedTexture{mDrawContext.mTextures.front(), GL_TEXTURE0 + gTextureUnit};

    glDrawArraysInstanced(GL_TRIANGLE_STRIP,
                          0,
                          gVerticesPerInstance,
                          static_cast<GLsizei>(mTiles.size()));
}

} // namespace graphics
} // namespace ad
