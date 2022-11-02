#include "Tiling.h"

#include "AppInterface.h"
#include "SpriteLoading.h"
#include "Vertex.h"

#include <handy/vector_utils.h>

#include <math/Range.h>
#include <math/Transformations.h>

#include <renderer/Uniforms.h>


namespace ad {
namespace graphics {


const GLchar* gTilingVertexShader = R"#(
    #version 400

    // The vertex position contains the cellsize.
    layout(location=0) in vec4  ve_VertexPosition;
    layout(location=1) in ivec2 in_UV;
    layout(location=2) in vec2  in_InstancePosition;
    layout(location=3) in ivec4 in_TextureArea;

    uniform ivec2 u_GridPosition;
    uniform mat3 u_camera;
    uniform mat3 u_projection;

    out vec2 ex_UV;

    void main(void)
    {
        vec3 worldPosition = vec3(in_InstancePosition + ve_VertexPosition.xy + u_GridPosition, 1.);
        vec3 transformed = u_projection * u_camera * worldPosition;
        gl_Position = vec4(transformed.x, transformed.y, 0., 1.);

        ex_UV = in_TextureArea.xy + (in_UV * in_TextureArea.zw);
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
    for (int y : math::range(aGridDefinition.height()))
    {
        for (int x : math::range(aGridDefinition.width()))
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
                                                               std::span{quad}));
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
                         std::span{positions},
                         1));

    // The tile sprite (as a LoadedSprite, i.e. the rectangle cutout in the image)
    specification.mVertexBuffers.push_back(
        initVertexBuffer<TileSet::Instance>(
            specification.mVertexArray,
            {
                { {3, Attribute::Access::Integer}, 4, 0, MappedGL<GLint>::enumerator}
            },
            1));

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

TileSet::TileSet(Size2<int> aCellSize, Size2<int> aGridDefinition) :
    mVertexSpecification{makeVertexGrid(aCellSize, aGridDefinition)},
    mTileSize{aCellSize},
    mGridDefinition{aGridDefinition},
    mGridRectangleScreen{{0.f, 0.f},
                          static_cast<Size2<TileSet::Position_t>>(aCellSize.cwMul(aGridDefinition))}
{}


void TileSet::resetTiling(Size2<int> aCellSize, Size2<int> aGridDefinition)
{
    bind(mVertexSpecification);

    std::vector<Vertex> quad = makeQuad(aCellSize);
    respecifyBuffer(mVertexSpecification.mVertexBuffers.at(0), std::span{quad});

    Vec2<int> cellOffset(aCellSize);
    std::vector<Position2<GLint>> positions = makePositions(cellOffset, aGridDefinition);
    respecifyBuffer(mVertexSpecification.mVertexBuffers.at(1), std::span{positions});

    mTileSize = aCellSize;
    mGridDefinition = aGridDefinition;
    mGridRectangleScreen.mDimension
        = static_cast<Size2<TileSet::Position_t>>(aCellSize.cwMul(aGridDefinition));
}


void TileSet::updateInstances(std::span<const Instance> aInstances)
{
    assert(aInstances.size() == getTileCount());
    //
    // Stream vertex attributes
    //
    respecifyBuffer(mVertexSpecification.mVertexBuffers.back(),
                    aInstances);
}


Tiling::Tiling() :
    mProgram{makeProgram()}
{
    setCameraTransformation(math::AffineMatrix<3, GLfloat>::Identity());
    setProjectionTransformation(math::AffineMatrix<3, GLfloat>::Identity());
}


void Tiling::render(const sprite::LoadedAtlas & aAtlas, const TileSet & aTileSet) const
{
    // The reason program data member is mutable...
    setUniform(mProgram, "u_GridPosition",
               static_cast<Position2<GLint>>(aTileSet.getPosition()));

    activate(aTileSet.mVertexSpecification, mProgram);

    //
    // Draw
    //
    ScopedBind scopedTexture{*aAtlas.texture, GL_TEXTURE0 + gTextureUnit};

    glDrawArraysInstanced(GL_TRIANGLE_STRIP,
                          0,
                          TileSet::gVerticesPerInstance,
                          aTileSet.getTileCount());
}


void Tiling::setCameraTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation)
{
    setUniform(mProgram, "u_camera", aTransformation); 
}


void Tiling::setProjectionTransformation(const math::Matrix<3, 3, GLfloat> & aTransformation)
{
    setUniform(mProgram, "u_projection", aTransformation); 
}


} // namespace graphics
} // namespace ad
