#include "TrivialShaping.h"

#include "shaders.h"

#include <renderer/Error.h>
#include <renderer/Uniforms.h>
#include <renderer/VertexSpecification.h>

#include <math/Transformations.h>


namespace ad {
namespace graphics {


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
        {0, {2, offsetof(VertexData, mPosition), MappedGL<GLfloat>::enumerator}},
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
        { 1,                                        {2,      offsetof(TrivialShaping::Rectangle, mGeometry.mPosition),  MappedGL<GLfloat>::enumerator}},
        { 2,                                        {2,      offsetof(TrivialShaping::Rectangle, mGeometry.mDimension), MappedGL<GLfloat>::enumerator}},
        { 3,                                        {{3, 3}, offsetof(TrivialShaping::Rectangle, mMatrixTransform),     MappedGL<GLfloat>::enumerator}},
        {{6, ShaderParameter::Access::Float, true}, {4,      offsetof(TrivialShaping::Rectangle, mColor),               MappedGL<GLubyte>::enumerator}},
    };

    VertexSpecification make_VertexSpecification()
    {
        VertexSpecification specification;
        appendToVertexSpecification(specification, gVertexDescription, std::span{gVertices});
        appendToVertexSpecification(specification, gInstanceDescription, std::span<TrivialShaping::Rectangle>{}, 1);
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


TrivialShaping::RectangleAngle::operator TrivialShaping::Rectangle ()
{
    return Rectangle{
        mGeometry,
        mColor,
        math::trans2d::rotateAbout(angle, center),
    };
}


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


void TrivialShaping::updateInstances(std::span<const TrivialShaping::Rectangle> aInstances)
{
    //
    // Stream vertex attributes
    //
    // The last vertex buffer added to the specification is the per instance data.
    respecifyBuffer(mDrawContext.mVertexSpecification.mVertexBuffers.back(),
                    aInstances);
    mInstanceCount = (GLsizei)aInstances.size();
}


void TrivialShaping::render() const
{
    activate(mDrawContext);

    //
    // Draw
    //
    glDrawArraysInstanced(GL_TRIANGLE_STRIP,
                          0,
                          gVerticesCount,
                          static_cast<GLsizei>(mInstanceCount));
}


void TrivialShaping::setCameraTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation)
{
    setUniform(mDrawContext.mProgram, "u_camera", aTransformation);
}


void TrivialShaping::setProjectionTransformation(const math::Matrix<3, 3, GLfloat> & aTransformation)
{
    setUniform(mDrawContext.mProgram, "u_projection", aTransformation);
}


} // namespace graphics
} // namespace ad
