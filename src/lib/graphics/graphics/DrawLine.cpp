#include "DrawLine.h"

#include "shaders.h"

#include <renderer/Error.h>
#include <renderer/VertexSpecification.h>


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
        { 1,                                  2, offsetof(DrawLine::Line, mOrigin),     MappedGL<GLint>::enumerator},
        { 2,                                  2, offsetof(DrawLine::Line, mEnd),        MappedGL<GLint>::enumerator},
        { 3,                                  1, offsetof(DrawLine::Line, width),        MappedGL<GLfloat>::enumerator},
        {{4, Attribute::Access::Float, true}, 3, offsetof(DrawLine::Line, mColor),      MappedGL<GLubyte>::enumerator},
    };

    VertexSpecification make_VertexSpecification()
    {
        VertexSpecification specification;
        appendToVertexSpecification(specification, gVertexDescription,   gsl::span{gVertices});
        appendToVertexSpecification<DrawLine::Line>(specification, gInstanceDescription, {}, 1);
        return specification;
    }

    Program make_Program()
    {
        return makeLinkedProgram({
                  {GL_VERTEX_SHADER, gSolidColorLineVertexShader},
                  {GL_FRAGMENT_SHADER, gTrivialFragmentShader},
               });
    }

} // anonymous namespace


DrawLine::DrawLine(std::shared_ptr<AppInterface> aAppInterface) :
    mDrawContext{
        make_VertexSpecification(),
        make_Program()
    },
    mListenWindowSize{aAppInterface->listenWindowResize(
        std::bind(&DrawLine::setWindowResolution, this, std::placeholders::_1))}
{
    setWindowResolution(aAppInterface->getWindowSize());
}


void DrawLine::clearShapes()
{
    mInstances.clear();
}


void DrawLine::addLine(Line aLineData)
{
    mInstances.push_back(std::move(aLineData));
}


void DrawLine::render() const
{
    activate(mDrawContext);

    //
    // Stream vertex attributes
    //

    // The last vertex buffer added to the specification is the per instance data.
    respecifyBuffer(mDrawContext.mVertexSpecification.mVertexBuffers.back(),
                    gsl::span<const Line>{mInstances});

    //
    // Draw
    //
    glDrawArraysInstanced(GL_TRIANGLE_STRIP,
                          0,
                          gVerticesCount,
                          static_cast<GLsizei>(mInstances.size()));
}


void DrawLine::setWindowResolution(Size2<int> aNewResolution)
{
    GLint location = glGetUniformLocation(mDrawContext.mProgram, "in_BufferResolution");
    glProgramUniform2iv(mDrawContext.mProgram, location, 1, aNewResolution.data());
}


} // namespace graphics
} // namespace ad
