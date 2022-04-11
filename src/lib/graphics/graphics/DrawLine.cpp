#include "DrawLine.h"

#include "shaders.h"

#include <renderer/Error.h>
#include <renderer/Uniforms.h>
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
    // Note: X is used for origin(0) or end(1).
    // Y is used for width. +normal(1) or -normal(-1).
    constexpr VertexData gVertices[gVerticesCount]{
        {{0.0f,  1.0f}},
        {{0.0f, -1.0f}},
        {{1.0f,  1.0f}},
        {{1.0f, -1.0f}},
    };

    //
    // Per instance data
    //
    constexpr AttributeDescriptionList gInstanceDescription{
        { 1,                                  3, offsetof(DrawLine::Line, mOrigin),       MappedGL<GLfloat>::enumerator},
        { 2,                                  3, offsetof(DrawLine::Line, mEnd),          MappedGL<GLfloat>::enumerator},
        { 3,                                  1, offsetof(DrawLine::Line, mWidth_screen), MappedGL<GLfloat>::enumerator},
        {{4, Attribute::Access::Float, true}, 4, offsetof(DrawLine::Line, mColor),        MappedGL<GLubyte>::enumerator},
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
                  {GL_VERTEX_SHADER, drawline::gSolidColorLineVertexShader},
                  {GL_FRAGMENT_SHADER, gTrivialFragmentShaderOpacity},
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
    setCameraTransformation(math::AffineMatrix<4, GLfloat>::Identity());
    setProjectionTransformation(math::AffineMatrix<4, GLfloat>::Identity());
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

    auto scopedDepthTest = graphics::scopeFeature(GL_DEPTH_TEST, false);
    auto scopedBlend = graphics::scopeFeature(GL_BLEND, true);

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


void DrawLine::setCameraTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation)
{
    setUniform(mDrawContext.mProgram, "u_view", aTransformation); 
}


void DrawLine::setProjectionTransformation(const math::Matrix<4, 4, GLfloat> & aTransformation)
{
    setUniform(mDrawContext.mProgram, "u_projection", aTransformation); 
}


void DrawLine::setWindowResolution(Size2<int> aNewResolution)
{
    GLint location = glGetUniformLocation(mDrawContext.mProgram, "in_BufferResolution");
    glProgramUniform2iv(mDrawContext.mProgram, location, 1, aNewResolution.data());
}


} // namespace graphics
} // namespace ad
