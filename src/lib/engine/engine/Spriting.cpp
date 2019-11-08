#include "Spriting.h"

#include "shaders.h"
#include "Vertex.h"

#include <renderer/Texture.h>

#include <handy/vector_utils.h>


namespace ad {


// Note: texture_2D_rect indices are texel based (not normalized)
constexpr size_t gVerticesCount{4};
Vertex gVerticesQuad[gVerticesCount] = {
    Vertex{
        {0.0f, 0.0f},
        {0, 0},
    },
    Vertex{
        {0.0f,  1.0f},
        {0, 1},
    },
    Vertex{
        { 1.0f, 0.0f},
        {1, 0},
    },
    Vertex{
        { 1.0f,  1.0f},
        {1, 1},
    },
};

namespace {

VertexSpecification makeQuad()
{
    VertexSpecification specification;
    glBindVertexArray(specification.mVertexArray);

    using namespace vertex; // for vertex::attr()

    // Per-vertex attributes
    specification.mVertexBuffers.emplace_back(
        makeLoadedVertexBuffer(
            {
                { 0,                               2, offsetof(Vertex, mPosition), MappedGL<GLfloat>::enumerator},
                { {1, Attribute::Access::Integer}, 2, offsetof(Vertex, mUV),       MappedGL<GLint>::enumerator},
            },
            sizeof(Vertex),
            sizeof(gVerticesQuad),
            gVerticesQuad
        ));
        /// \todo For that to work, the array_utils have to work with math type derived from MatrixBase
        //makeLoadedVertexBuffer(range(gVerticesQuad),
        //                       attr(0,                               &Vertex::mPosition),
        //                       attr({1, Attribute::Access::Integer}, &Vertex::mUV)));

    // Per-instance attributes
    specification.mVertexBuffers.push_back(
        makeLoadedVertexBuffer(
            {
                { 2,                               2, offsetof(Instance, mPosition),    MappedGL<GLint>::enumerator},
                { {3, Attribute::Access::Integer}, 4, offsetof(Instance, mTextureArea), MappedGL<GLint>::enumerator},
            },
            0,
            0,
            nullptr
        ));
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);

    return specification;
}


Program makeProgram()
{
    Program program = makeLinkedProgram({
                          {GL_VERTEX_SHADER, gVertexShader},
                          {GL_FRAGMENT_SHADER, gAnimationFragmentShader},
                      });

    glProgramUniform1i(program, glGetUniformLocation(program, "spriteSampler"), 0);

    return program;
}

} // anonymous namespace

Spriting::Spriting(Size2<int> aRenderResolution) :
        mDrawContext(makeQuad(), makeProgram()),
        mSprites()
{
    setBufferResolution(aRenderResolution);
}


void Spriting::render() const
{
    activate(mDrawContext);

    //
    // Stream vertex attributes
    //
    respecifyBuffer(mDrawContext.mVertexSpecification.mVertexBuffers.back(),
                    mSprites.data(),
                    static_cast<GLsizei>(getStoredSize(mSprites)));

    //
    // Draw
    //
    glDrawArraysInstanced(GL_TRIANGLE_STRIP,
                          0,
                          gVerticesCount,
                          static_cast<GLsizei>(mSprites.size()));
}


void Spriting::setBufferResolution(Size2<int> aNewResolution)
{
    GLint location = glGetUniformLocation(mDrawContext.mProgram, "in_BufferResolution");
    glProgramUniform2iv(mDrawContext.mProgram, location, 1, aNewResolution.data());
}


} // namespace ad
