#include "TrivialPolygon.h"

#include "shaders.h"

#include <renderer/Error.h>
#include <renderer/Uniforms.h>
#include <renderer/VertexSpecification.h>

#include <math/Transformations.h>


namespace ad {
namespace graphics {


namespace 
{
    using PolygonPoint = TrivialPolygon::PolygonPoint;

    //
    // Per vertex data
    //
    constexpr AttributeDescriptionList gVertexDescription{
        { 0,                                  2, offsetof(PolygonPoint, mPosition), MappedGL<GLfloat>::enumerator},
        {{1, Attribute::Access::Float, true}, 3, offsetof(PolygonPoint, mColor),    MappedGL<GLubyte>::enumerator},
        { 3,                                  3, offsetof(PolygonPoint, mMatrixTransform), MappedGL<GLfloat>::enumerator},
        { 4,                                  3, offsetof(PolygonPoint, mMatrixTransform) + 3 * sizeof(GLfloat), MappedGL<GLfloat>::enumerator},
        { 5,                                  3, offsetof(PolygonPoint, mMatrixTransform) + 6 * sizeof(GLfloat), MappedGL<GLfloat>::enumerator},
    };

    VertexSpecification make_VertexSpecification()
    {
        VertexSpecification specification;
        appendToVertexSpecification(specification, gVertexDescription, gsl::span<PolygonPoint>{});
        return specification;
    }

    Program make_Program()
    {
        return makeLinkedProgram({
                  {GL_VERTEX_SHADER, gTrivialColorVertexShader},
                  {GL_FRAGMENT_SHADER, gTrivialFragmentShader},
               });
    }

} // anonymous namespace


TrivialPolygon::TrivialPolygon(Size2<int> aRenderResolution) :
    mDrawContext{
        make_VertexSpecification(),
        make_Program()
    },
    mIbo{ makeLoadedIndexBuffer(gsl::span<Index>{}, BufferHint::StreamDraw) }
{
    setCameraTransformation(math::AffineMatrix<3, GLfloat>::Identity());
    setProjectionTransformation(math::trans2d::window<GLfloat>(
        {{0.f, 0.f}, static_cast<math::Size<2, GLfloat>>(aRenderResolution)},
        {{-1.f, -1.f}, {2.f, 2.f}}
    ));
}


void TrivialPolygon::clearPolygons()
{
    mVertexAttributes.clear();
    mIndices.clear();
    mNextIndex = 0;
}


void TrivialPolygon::addVertices(std::initializer_list<PolygonPoint> aPoints)
{
    addVertices(aPoints.begin(), aPoints.end());
}

void TrivialPolygon::addVertices(std::vector<PolygonPoint> aPoints)
{
    addVertices(aPoints.begin(), aPoints.end());
}


void TrivialPolygon::render()
{
    activate(mDrawContext);

    //
    // Stream vertex attributes
    //
    respecifyBuffer<PolygonPoint>(mDrawContext.mVertexSpecification.mVertexBuffers.front(),
                               mVertexAttributes);

    //
    // Stream index buffer
    //
    respecifyBuffer<Index>(mIbo, mIndices);

    //
    // Draw
    //
    {
        auto restartIndexGuard = scopePrimitiveRestartIndex(gRestartIndex);

        glDrawElements(GL_TRIANGLE_FAN,
                       static_cast<GLsizei>(mIndices.size()),
                       MappedGL<Index>::enumerator,
                       static_cast<void *>(0));
    }
}


void TrivialPolygon::setCameraTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation)
{
    setUniform(mDrawContext.mProgram, "camera", aTransformation); 
}


void TrivialPolygon::setProjectionTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation)
{
    setUniform(mDrawContext.mProgram, "projection", aTransformation); 
}


} // namespace graphics
} // namespace ad
