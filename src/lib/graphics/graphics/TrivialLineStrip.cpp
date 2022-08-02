#include "TrivialLineStrip.h"

#include "shaders.h"

#include <renderer/Error.h>
#include <renderer/Uniforms.h>
#include <renderer/VertexSpecification.h>

#include <math/Transformations.h>


namespace ad {
namespace graphics {


namespace 
{
    using LinePoint = TrivialLineStrip::LinePoint;

    //
    // Per vertex data
    //
    constexpr AttributeDescriptionList gVertexDescription{
        { 0,                                  2, offsetof(LinePoint, mPosition), MappedGL<GLfloat>::enumerator},
        {{1, Attribute::Access::Float, true}, 3, offsetof(LinePoint, mColor),    MappedGL<GLubyte>::enumerator},
    };

    VertexSpecification make_VertexSpecification()
    {
        VertexSpecification specification;
        appendToVertexSpecification(specification, gVertexDescription, std::span<LinePoint>{});
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


TrivialLineStrip::TrivialLineStrip(Size2<int> aRenderResolution) :
    mDrawContext{
        make_VertexSpecification(),
        make_Program()
    },
    mIbo{ loadIndexBuffer(mDrawContext.mVertexSpecification.mVertexArray,
                          std::span<Index>{}, BufferHint::StreamDraw) }
{
    setCameraTransformation(math::AffineMatrix<3, GLfloat>::Identity());
    setProjectionTransformation(math::trans2d::window<GLfloat>(
        {{0.f, 0.f}, static_cast<math::Size<2, GLfloat>>(aRenderResolution)},
        {{-1.f, -1.f}, {2.f, 2.f}}
    ));
}


void TrivialLineStrip::clearLines()
{
    mVertexAttributes.clear();
    mIndices.clear();
    mNextIndex = 0;
}


void TrivialLineStrip::addLine(std::initializer_list<LinePoint> aPoints)
{
    addLine(aPoints.begin(), aPoints.end());
}


void TrivialLineStrip::outlineRectangle(const Rectangle<GLfloat> & aRectangle, const Color aColor)
{
    addLine({
        {aRectangle.bottomLeft(), aColor},
        {aRectangle.topLeft(), aColor},
        {aRectangle.topRight(), aColor},
        {aRectangle.bottomRight(), aColor},
        {aRectangle.bottomLeft(), aColor}
    });
}

void TrivialLineStrip::render() const
{
    activate(mDrawContext);

    //
    // Stream vertex attributes
    //
    respecifyBuffer(mDrawContext.mVertexSpecification.mVertexBuffers.front(),
                    std::span{mVertexAttributes});

    //
    // Stream index buffer
    //
    respecifyBuffer(mIbo, std::span{mIndices});

    //
    // Draw
    //
    {
        auto restartIndexGuard = scopePrimitiveRestartIndex(gRestartIndex);

        glDrawElements(GL_LINE_STRIP,
                       static_cast<GLsizei>(mIndices.size()),
                       MappedGL<Index>::enumerator,
                       static_cast<void *>(0));
    }
}


void TrivialLineStrip::setCameraTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation)
{
    setUniform(mDrawContext.mProgram, "camera", aTransformation); 
}


void TrivialLineStrip::setProjectionTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation)
{
    setUniform(mDrawContext.mProgram, "projection", aTransformation); 
}


} // namespace graphics
} // namespace ad
