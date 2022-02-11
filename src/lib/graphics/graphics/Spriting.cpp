#include "Spriting.h"

#include "shaders.h"
#include "SpriteLoading.h"
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
        initVertexBuffer<Spriting::Instance>(
            specification.mVertexArray,
            {
                // Model transform
                { 2, 3, offsetof(Spriting::Instance, mModelTransform) + 0 * sizeof(GLfloat), MappedGL<GLfloat>::enumerator},
                { 3, 3, offsetof(Spriting::Instance, mModelTransform) + 3 * sizeof(GLfloat), MappedGL<GLfloat>::enumerator},
                { 4, 3, offsetof(Spriting::Instance, mModelTransform) + 6 * sizeof(GLfloat), MappedGL<GLfloat>::enumerator},
                // LoadedSprite (i.e. sprite rectangle cutout in the texture)
                { {5, Attribute::Access::Integer}, 4, offsetof(Spriting::Instance, mLoadedSprite),  MappedGL<GLint>::enumerator},
                { 6,                               1, offsetof(Spriting::Instance, mOpacity),       MappedGL<GLfloat>::enumerator},
                { 7,                               2, offsetof(Spriting::Instance, mAxisMirroring), MappedGL<GLint>::enumerator},
            },
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


Spriting::Instance::Instance(math::AffineMatrix<3, GLfloat> aModelTransform,
                             LoadedSprite aSprite,
                             GLfloat aOpacity,
                             Mirroring aMirroring) :
    mModelTransform{aModelTransform},
    mLoadedSprite{std::move(aSprite)},
    mOpacity{aOpacity},
    mAxisMirroring{
        (test(aMirroring, Mirroring::FlipHorizontal) ? -1 : 1),
        (test(aMirroring, Mirroring::FlipVertical) ? -1 : 1)
    }
{}


Spriting::Instance::Instance(Position2<GLfloat> aRenderingPosition, 
                             LoadedSprite aSprite,
                             GLfloat aOpacity,
                             Mirroring aMirroring) :
    Instance{math::trans2d::translate(aRenderingPosition.as<math::Vec>()), 
             aSprite,
             aOpacity,
             aMirroring}
{}


Spriting::Spriting(GLfloat aPixelSize) :
        mVertexSpecification{makeQuad()},
        mProgram{makeProgram()}
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
    respecifyBuffer(mVertexSpecification.mVertexBuffers.back(),
                    aInstances);
    mInstanceCount = static_cast<GLsizei>(aInstances.size());
}


void Spriting::render(const sprite::LoadedAtlas & aAtlas) const
{
    activate(mVertexSpecification, mProgram);

    bind_guard scopedTexture{*aAtlas.texture, GL_TEXTURE0 + gTextureUnit};

    glDrawArraysInstanced(GL_TRIANGLE_STRIP,
                          0,
                          gVerticesCount,
                          mInstanceCount);
}


void Spriting::setPixelWorldSize(GLfloat aPixelSize)
{
    setUniform(mProgram, "u_pixelWorldSize", math::Vec<2, GLfloat>{aPixelSize, aPixelSize}); 
}


void Spriting::setCameraTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation)
{
    setUniform(mProgram, "u_camera", aTransformation); 
}


void Spriting::setProjectionTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation)
{
    setUniform(mProgram, "u_projection", aTransformation); 
}


} // namespace graphics
} // namespace ad
