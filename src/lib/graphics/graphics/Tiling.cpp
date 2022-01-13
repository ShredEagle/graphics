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

    layout(location=0) in vec4  in_VertexPosition;
    layout(location=1) in ivec2 in_UV;
    layout(location=2) in vec2  in_InstancePosition;
    layout(location=3) in ivec4 in_TextureArea;

    uniform ivec2 u_GridPosition;
    uniform mat3 u_camera;
    uniform mat3 u_projection;

    out vec2 ex_UV;

    void main(void)
    {
        vec3 worldPosition = vec3(in_InstancePosition + in_VertexPosition.xy + u_GridPosition, 1.);
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
                         gsl::make_span(positions),
                         1));

    // The tile sprite (as a LoadedSprite, i.e. the rectangle cutout in the image)
    /// \todo separate buffer specification and filling
    specification.mVertexBuffers.push_back(
        initVertexBuffer<Tiling::Instance>(
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


Tiling::Tiling(Size2<int> aCellSize, Size2<int> aGridDefinition) :
    mVertexSpecification{makeVertexGrid(aCellSize, aGridDefinition)},
    mProgram{makeProgram()},
    mTileSize{aCellSize},
    mGridDefinition{aGridDefinition},
    mGridRectangleScreen{{0.f, 0.f},
                          static_cast<Size2<Tiling::Position_t>>(aCellSize.cwMul(aGridDefinition))}
{
    setCameraTransformation(math::AffineMatrix<3, GLfloat>::Identity());
    setProjectionTransformation(math::AffineMatrix<3, GLfloat>::Identity());
}


void Tiling::resetTiling(Size2<int> aCellSize, Size2<int> aGridDefinition)
{
    bindVertexArray(mVertexSpecification);

    std::vector<Vertex> quad = makeQuad(aCellSize);
    respecifyBuffer(mVertexSpecification.mVertexBuffers.at(0), gsl::make_span(quad));

    Vec2<int> cellOffset(aCellSize);
    std::vector<Position2<GLint>> positions = makePositions(cellOffset, aGridDefinition);
    respecifyBuffer(mVertexSpecification.mVertexBuffers.at(1), gsl::make_span(positions));

    mTileSize = aCellSize;
    mGridDefinition = aGridDefinition;
    mGridRectangleScreen.mDimension
        = static_cast<Size2<Tiling::Position_t>>(aCellSize.cwMul(aGridDefinition));
}


//void Tiling::setBufferResolution(Size2<int> aNewResolution)
//{
//    setUniform(mProgram, "in_BufferResolution", aNewResolution);
//}


void Tiling::setPosition(Position2<Position_t> aPosition)
{
    mGridRectangleScreen.mPosition = aPosition;
    setUniform(mProgram, "u_GridPosition", static_cast<Position2<GLint>>(aPosition));
}


void Tiling::load(const sprites::LoadedAtlas & aAtlas)
{
    mAtlasTexture = aAtlas.texture;
}


void Tiling::updateInstances(gsl::span<const Instance> aInstances)
{
    assert(aInstances.size() == getTileCount());
    //
    // Stream vertex attributes
    //
    respecifyBuffer(mVertexSpecification.mVertexBuffers.back(),
                    aInstances);
}


void Tiling::render() const
{
    activate(mVertexSpecification, mProgram);

    //
    // Draw
    //
    bind_guard scopedTexture{*mAtlasTexture, GL_TEXTURE0 + gTextureUnit};

    glDrawArraysInstanced(GL_TRIANGLE_STRIP,
                          0,
                          gVerticesPerInstance,
                          getTileCount());
}


void Tiling::setCameraTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation)
{
    setUniform(mProgram, "u_camera", aTransformation); 
}


void Tiling::setProjectionTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation)
{
    setUniform(mProgram, "u_projection", aTransformation); 
}


} // namespace graphics
} // namespace ad
