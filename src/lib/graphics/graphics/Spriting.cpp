#include "Spriting.h"

#include "shaders.h"
#include "Vertex.h"

#include <renderer/Texture.h>
#include <renderer/Uniforms.h>

#include <handy/vector_utils.h>

#include <math/Transformations.h>


namespace ad {
namespace graphics {


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

    // Per-vertex attributes
    specification.mVertexBuffers.emplace_back(
        loadVertexBuffer(
            specification.mVertexArray,
            {
                // Postion
                { 0,                               2, offsetof(Vertex, mPosition), MappedGL<GLfloat>::enumerator},
                // UV
                { {1, Attribute::Access::Integer}, 2, offsetof(Vertex, mUV),       MappedGL<GLint>::enumerator},
            },
            sizeof(Vertex),
            sizeof(gVerticesQuad),
            gVerticesQuad
        ));
        /// TODO For that to work, the array_utils have to work with math type derived from MatrixBase
        //using namespace vertex; // for vertex::attr()
        //makeLoadedVertexBuffer(range(gVerticesQuad),
        //                       attr(0,                               &Vertex::mPosition),
        //                       attr({1, Attribute::Access::Integer}, &Vertex::mUV)));

    // Per-instance attributes
    specification.mVertexBuffers.push_back(
        loadVertexBuffer(
            specification.mVertexArray,
            {
                // Sprite position
                { 2,                               2, offsetof(Spriting::Instance, mPosition),    MappedGL<GLfloat>::enumerator},
                // LoadedSprite (i.e. sprite rectangle cutout in the texture)
                { {3, Attribute::Access::Integer}, 4, offsetof(Spriting::Instance, mLoadedSprite), MappedGL<GLint>::enumerator},
                { 4,                               1, offsetof(Spriting::Instance, mOpacity),      MappedGL<GLfloat>::enumerator},
            },
            0,
            0,
            nullptr,
            1
        ));

    return specification;
}


Program makeProgram()
{
    Program program = makeLinkedProgram({
                          {GL_VERTEX_SHADER,   gSpriteVertexShader},
                          {GL_FRAGMENT_SHADER, gAnimationFragmentShader},
                      });

    // Matches GL_TEXTURE0 from Spriting::load
    glProgramUniform1i(program, glGetUniformLocation(program, "spriteSampler"), Spriting::gTextureUnit);

    return program;
}

} // anonymous namespace


Spriting::Spriting(GLfloat aPixelSize) :
        mDrawContext{makeQuad(), makeProgram()}
{
    setPixelWorldSize(aPixelSize);

    setCameraTransformation(math::AffineMatrix<3, GLfloat>::Identity());
    setProjectionTransformation(math::AffineMatrix<3, GLfloat>::Identity());
}


void Spriting::updateInstances(gsl::span<const Instance> aInstances)
{
    //
    // Stream vertex attributes
    //
    respecifyBuffer(mDrawContext.mVertexSpecification.mVertexBuffers.back(),
                    aInstances);
    mInstanceCount = static_cast<GLsizei>(aInstances.size());
}


void Spriting::render() const
{
    activate(mDrawContext);

    bind_guard scopedTexture{mDrawContext.mTextures.front(), GL_TEXTURE0 + gTextureUnit};

    glDrawArraysInstanced(GL_TRIANGLE_STRIP,
                          0,
                          gVerticesCount,
                          mInstanceCount);
}


void Spriting::setPixelWorldSize(GLfloat aPixelSize)
{
    setUniform(mDrawContext.mProgram, "u_pixelWorldSize", math::Vec<2, GLfloat>{aPixelSize, aPixelSize}); 
}


void Spriting::setCameraTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation)
{
    setUniform(mDrawContext.mProgram, "u_camera", aTransformation); 
}


void Spriting::setProjectionTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation)
{
    setUniform(mDrawContext.mProgram, "u_projection", aTransformation); 
}


void Spriting::setViewportVirtualResolution(math::Size<2, int> aViewportPixelSize)
{
    setProjectionTransformation(
        math::trans2d::orthographicProjection<GLfloat>(math::Rectangle<GLfloat>{
            math::Position<2, GLfloat>::Zero(),
            static_cast<math::Size<2, GLfloat>>(aViewportPixelSize)
        }.centered()));
}


} // namespace graphics
} // namespace ad
