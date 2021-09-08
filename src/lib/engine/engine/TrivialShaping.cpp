#include "TrivialShaping.h"

#include "shaders.h"

#include <renderer/Error.h>
#include <renderer/Uniforms.h>
#include <renderer/VertexSpecification.h>

#include <math/Transformations.h>


namespace ad {


namespace 
{

    //
    // Per vertex data
    //
    struct VertexData
    {
        Position2<GLfloat> mPosition;
    };

    constexpr AttributeDescriptionList gVertexDescription{
        {0, 2, offsetof(VertexData, mPosition), MappedGL<GLfloat>::enumerator},
    };

    constexpr std::size_t gVerticesCount = 4;
    constexpr VertexData gVertices[gVerticesCount]{
        {{0.0f, 0.0f}},
        {{0.0f, 1.0f}},
        {{1.0f, 0.0f}},
        {{1.0f, 1.0f}},
    };

    //
    // Per instance data
    //
    constexpr AttributeDescriptionList gInstanceDescription{
        { 1,                                  2, offsetof(TrivialShaping::Rectangle, mGeometry.mPosition),  MappedGL<GLfloat>::enumerator},
        { 2,                                  2, offsetof(TrivialShaping::Rectangle, mGeometry.mDimension), MappedGL<GLfloat>::enumerator},
        { 3,                                  3, offsetof(TrivialShaping::Rectangle, mMatrixTransform), MappedGL<GLfloat>::enumerator},
        { 4,                                  3, offsetof(TrivialShaping::Rectangle, mMatrixTransform) + 3 * sizeof(GLfloat), MappedGL<GLfloat>::enumerator},
        { 5,                                  3, offsetof(TrivialShaping::Rectangle, mMatrixTransform) + 6 * sizeof(GLfloat), MappedGL<GLfloat>::enumerator},
        {{6, Attribute::Access::Float, true}, 3, offsetof(TrivialShaping::Rectangle, mColor),               MappedGL<GLubyte>::enumerator},
    };

    VertexSpecification make_VertexSpecification()
    {
        VertexSpecification specification;
        appendToVertexSpecification(specification, gVertexDescription,   gsl::span{gVertices});
        appendToVertexSpecification<TrivialShaping::Rectangle>(specification, gInstanceDescription, {}, 1);
        return specification;
    }

    Program make_Program()
    {
        return makeLinkedProgram({
                  {GL_VERTEX_SHADER, gSolidColorInstanceVertexShader},
                  {GL_FRAGMENT_SHADER, gTrivialFragmentShader},
               });
    }

} // anonymous namespace


TrivialShaping::TrivialShaping(Size2<int> aRenderResolution) :
    mDrawContext{
        make_VertexSpecification(),
        make_Program()
    }
{
    setCameraTransformation(math::AffineMatrix<3, GLfloat>::Identity());
    setProjectionTransformation(math::trans2d::window<GLfloat>(
        {{0.f, 0.f}, static_cast<math::Size<2, GLfloat>>(aRenderResolution)},
        {{-1.f, -1.f}, {2.f, 2.f}}
    ));
}


void TrivialShaping::clearShapes()
{
    mInstances.clear();
}


void TrivialShaping::addRectangle(Rectangle aRectangleData)
{
    mInstances.push_back(std::move(aRectangleData));
}


void TrivialShaping::render()
{
    activate(mDrawContext);

    //
    // Stream vertex attributes
    //

    // The last vertex buffer added to the specification is the per instance data.
    respecifyBuffer(mDrawContext.mVertexSpecification.mVertexBuffers.back(),
                    gsl::span<Rectangle>{mInstances});

    //
    // Draw
    //
    glDrawArraysInstanced(GL_TRIANGLE_STRIP,
                          0,
                          gVerticesCount,
                          static_cast<GLsizei>(mInstances.size()));
}


void TrivialShaping::setCameraTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation)
{
    setUniform(mDrawContext.mProgram, "camera", aTransformation); 
}


void TrivialShaping::setProjectionTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation)
{
    setUniform(mDrawContext.mProgram, "projection", aTransformation); 
}


} // namespace ad
