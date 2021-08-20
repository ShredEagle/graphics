#include "TrivialLineStrip.h"

#include "shaders.h"

#include <renderer/Error.h>
#include <renderer/VertexSpecification.h>


namespace ad {


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
        appendToVertexSpecification(specification, gVertexDescription, gsl::span<LinePoint>{});
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
    mIbo{ makeLoadedIndexBuffer(gsl::span<Index>{}, BufferHint::StreamDraw) }
{
    setBufferResolution(aRenderResolution);
}


void TrivialLineStrip::clearLines()
{
    mVertexAttributes.clear();
    mIndices.clear();
    mNextIndex = 0;
}


void TrivialLineStrip::addLine(std::initializer_list<LinePoint> aPoints)
{
    std::move(aPoints.begin(), aPoints.end(),
              std::back_inserter(mVertexAttributes));

    for (auto _ : aPoints)
    {
        mIndices.push_back(mNextIndex++);
    }
    mIndices.push_back(gRestartIndex);
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

void TrivialLineStrip::render()
{
    activate(mDrawContext);

    //
    // Stream vertex attributes
    //
    respecifyBuffer<LinePoint>(mDrawContext.mVertexSpecification.mVertexBuffers.front(),
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

        glDrawElements(GL_LINE_STRIP,
                       static_cast<GLsizei>(mIndices.size()),
                       MappedGL<Index>::enumerator,
                       static_cast<void *>(0));
    }
}


void TrivialLineStrip::setBufferResolution(Size2<int> aNewResolution)
{
    GLint location = glGetUniformLocation(mDrawContext.mProgram, "in_BufferResolution");
    glProgramUniform2iv(mDrawContext.mProgram, location, 1, aNewResolution.data());
}


} // namespace ad
