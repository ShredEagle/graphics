#include "TrivialShaping.h"

#include "shaders.h"

#include <renderer/Error.h>
#include <renderer/VertexSpecification.h>


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
        { 1,                                  2, offsetof(TrivialShaping::Rectangle, mGeometry.mPosition),  MappedGL<GLint>::enumerator},
        {{2, Attribute::Access::Integer},     2, offsetof(TrivialShaping::Rectangle, mGeometry.mDimension), MappedGL<GLint>::enumerator},
        {{3, Attribute::Access::Float, true}, 3, offsetof(TrivialShaping::Rectangle, mColor),               MappedGL<GLubyte>::enumerator},
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
    setBufferResolution(aRenderResolution);
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


void TrivialShaping::setBufferResolution(Size2<int> aNewResolution)
{
    GLint location = glGetUniformLocation(mDrawContext.mProgram, "in_BufferResolution");
    glProgramUniform2iv(mDrawContext.mProgram, location, 1, aNewResolution.data());
}


} // namespace ad
