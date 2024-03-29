#include "Curving.h"

#include "Curving-shaders.h"

#include <renderer/Uniforms.h>

#include <math/Color.h>


namespace ad {
namespace graphics {


namespace {


    struct VertexData
    {
        GLfloat mParameter;
        GLbyte  mSide;
    };


    constexpr AttributeDescriptionList gVertexDescription{
        {0,                                     {1, offsetof(VertexData, mParameter),  MappedGL<GLfloat>::enumerator}},
        {{1, ShaderParameter::Access::Integer}, {1, offsetof(VertexData, mSide),       MappedGL<GLbyte>::enumerator}},
    };

    using Inst = Curving::Instance;

    constexpr AttributeDescriptionList gBezierInstanceDescription{
        { 2, {3, offsetof(Inst, bezier) + 0 * sizeof(Inst::Position_t), MappedGL<GLfloat>::enumerator}},
        { 3, {3, offsetof(Inst, bezier) + 1 * sizeof(Inst::Position_t), MappedGL<GLfloat>::enumerator}},
        { 4, {3, offsetof(Inst, bezier) + 2 * sizeof(Inst::Position_t), MappedGL<GLfloat>::enumerator}},
        { 5, {3, offsetof(Inst, bezier) + 3 * sizeof(Inst::Position_t), MappedGL<GLfloat>::enumerator}},
        { 6, {1, offsetof(Inst, startHalfWidth), MappedGL<GLfloat>::enumerator}},
        { 7, {1, offsetof(Inst,   endHalfWidth), MappedGL<GLfloat>::enumerator}},
        // Manual specification of a matrix over several attributes
        { 8, {4, offsetof(Inst, modelTransform) +  0 * sizeof(GLfloat), MappedGL<GLfloat>::enumerator}},
        { 9, {4, offsetof(Inst, modelTransform) +  4 * sizeof(GLfloat), MappedGL<GLfloat>::enumerator}},
        {10, {4, offsetof(Inst, modelTransform) +  8 * sizeof(GLfloat), MappedGL<GLfloat>::enumerator}},
        {11, {4, offsetof(Inst, modelTransform) + 12 * sizeof(GLfloat), MappedGL<GLfloat>::enumerator}},
    };


    std::vector<VertexData> generateVertices(std::size_t aCurveSegments)
    {
        std::vector<VertexData> vertices;
        for (std::size_t parameterStep = 0;
             parameterStep <= aCurveSegments;
            ++parameterStep)
        {
            GLfloat parameter = (GLfloat)parameterStep/(GLfloat)aCurveSegments;
            vertices.push_back({parameter, 1});
            vertices.push_back({parameter, -1});
        }
        return vertices;
    };


    VertexBufferObject makeCurveVertexBuffer(VertexArrayObject & aVAO, GLsizei aCurveSubdivisions)
    {
        auto vertices = generateVertices(aCurveSubdivisions);
        return loadVertexBuffer(aVAO, gVertexDescription, std::span{vertices}, BufferHint::StaticDraw);
    }

} // unnamed namespace


Curving::Curving(GLsizei aCurveSubdivisions, math::AffineMatrix<4, GLfloat> aProjectionTransformation) :
    mVertexCount{ 2 * (aCurveSubdivisions + 1) },
    mVertexBuffer{makeCurveVertexBuffer(mVertexArray, aCurveSubdivisions)},
    mInstanceBuffer{initVertexBuffer<Curving::Instance>(
        mVertexArray,
        gBezierInstanceDescription, 
        1)},
    mGpuProgram{makeLinkedProgram({
        {GL_VERTEX_SHADER,   curving::gVertexShader},
        {GL_FRAGMENT_SHADER, curving::gFragmentShader},
    })}
{
    setCameraTransformation(math::AffineMatrix<4, GLfloat>::Identity());
    setProjectionTransformation(aProjectionTransformation);

    setColor(math::hdr::gWhite<GLfloat>);
}


void Curving::render(std::span<const Instance> aInstances) const
{
    activate(mVertexArray, mGpuProgram);

    // Stream instance attributes
    respecifyBuffer(mInstanceBuffer, aInstances, BufferHint::StreamDraw);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP,
                          0,
                          mVertexCount,
                          static_cast<GLsizei>(aInstances.size()));
}


void Curving::setColor(math::hdr::Rgba_f aColor)
{
    setUniform(mGpuProgram, "u_color", aColor);
}


void Curving::setCameraTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation)
{
    setUniform(mGpuProgram, "u_camera", aTransformation); 
}


void Curving::setProjectionTransformation(const math::Matrix<4, 4, GLfloat> & aTransformation)
{
    setUniform(mGpuProgram, "u_projection", aTransformation); 
}


} // namespace graphics
} // namespace ad

