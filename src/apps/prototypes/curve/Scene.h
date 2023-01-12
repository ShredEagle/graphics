#pragma once


#include "shaders.h"

#include <graphics/Timer.h>

#include <renderer/Drawing.h>
#include <renderer/Uniforms.h>

#include <math/Transformations.h>
#include <math/Vector.h>
#include <math/VectorUtilities.h>


// The approach is to send N*2 parameters values in [0..1] as vertice data
// and send the bezier control points as instance data.
// Then use a vertex shader on each vertex to interpolate its position at its associated parameter value,
// offset by `[+/-] normal * width`.
// see: https://discourse.libcinder.org/t/smooth-efficient-perfect-curves/925

using namespace ad::graphics;

namespace ad {
namespace curve {


using Vec2 = math::Vec<2, GLfloat>;


struct CubicBezier
{
    std::array<Vec2, 4> p;
};


Vec2 deCasteljau(GLfloat t, CubicBezier b)
{
    // Uses the Bezier object as storage, hence the copy
    for (std::size_t degree = 0; degree != 3; ++degree)
    {
        for (std::size_t index = 0; index != 3-degree; ++index)
        {
            b.p[index] = (1-t) * b.p[index] + t * b.p[index+1];
        }
    }
    return b.p[0];
}


struct LineVertex
{
    Vec2 position;
};


constexpr AttributeDescriptionList gLineVertexDescription{
    {0, 2, offsetof(LineVertex, position), MappedGL<GLfloat>::enumerator},
};


struct GenerativeVertex
{
    GLfloat t;
    GLint side;
};


constexpr AttributeDescriptionList gGenerativeVertexDescription{
    {0,                               1, offsetof(GenerativeVertex, t),    MappedGL<GLfloat>::enumerator},
    {{1, ShaderParameter::Access::Integer}, 1, offsetof(GenerativeVertex, side), MappedGL<GLint>::enumerator},
};


constexpr AttributeDescriptionList gBezierInstanceDescription{
    {2, 2, offsetof(CubicBezier, p) + 0 * sizeof(Vec2), MappedGL<GLfloat>::enumerator},
    {3, 2, offsetof(CubicBezier, p) + 1 * sizeof(Vec2), MappedGL<GLfloat>::enumerator},
    {4, 2, offsetof(CubicBezier, p) + 2 * sizeof(Vec2), MappedGL<GLfloat>::enumerator},
    {5, 2, offsetof(CubicBezier, p) + 3 * sizeof(Vec2), MappedGL<GLfloat>::enumerator},
};


struct Scene
{
    Scene();

    void step(const Timer & aTimer);
    void render(const Size2<int> aRenderSize);

    constexpr static int gSubdivisions = 80;
    
    // Used to draw the bezier directly as a GL line
    // (after subdivision on the CPU)
    VertexSpecification mLineVertexSpecification;
    Program  mLineProgram;

    VertexSpecification mGenerativeVertexSpecification;
    Program  mGenerativeProgram;

    std::vector<LineVertex> mLineVertices;
    CubicBezier mBezier{ {Vec2{-0.8f, -0.5f}, Vec2{-0.3f, 1.8f}, Vec2{0.3f, -1.8f}, Vec2{0.8f, 0.5f}} };
};


const std::vector<GenerativeVertex> gGenerativeVertices = []()
{
    std::vector<GenerativeVertex> vertices;
    for (std::size_t i = 0; i <= Scene::gSubdivisions; ++i)
    {
        vertices.push_back({
            (GLfloat)i/Scene::gSubdivisions, 
            1,
        });
        vertices.push_back({
            (GLfloat)i/Scene::gSubdivisions, 
            -1,
        });
    }
    return vertices;
}();


inline Scene::Scene() :
    mLineProgram{makeLinkedProgram({
        {GL_VERTEX_SHADER,   gLineVertexShader},
        {GL_FRAGMENT_SHADER, gFragmentShader},
    })},
    mGenerativeProgram{makeLinkedProgram({
        {GL_VERTEX_SHADER,   gGenerativeVertexShader},
        {GL_FRAGMENT_SHADER, gFragmentShader},
    })}
{
    //
    // Line bezier
    //
    appendToVertexSpecification(mLineVertexSpecification,
                                gLineVertexDescription,
                                std::span<const LineVertex>{});

    for (std::size_t i = 0; i <= gSubdivisions; ++i)
    {
        mLineVertices.push_back(LineVertex{deCasteljau((GLfloat)i/gSubdivisions, mBezier)});
    }

    respecifyBuffer(mLineVertexSpecification.mVertexBuffers[0], std::span{mLineVertices});

    //
    // Fat bezier
    //
    appendToVertexSpecification(
        mGenerativeVertexSpecification,
        gGenerativeVertexDescription,
        std::span{gGenerativeVertices});

    appendToVertexSpecification(
        mGenerativeVertexSpecification,
        gBezierInstanceDescription,
        std::span<CubicBezier>{},
        1);
}


inline void Scene::step(const Timer & aTimer)
{
    setUniform(mGenerativeProgram, "halfWidth", 0.1f);
    respecifyBuffer(mGenerativeVertexSpecification.mVertexBuffers[1],
                    std::span{mBezier.p});
}


inline void Scene::render(const Size2<int> aRenderSize)
{
    math::AffineMatrix<4, GLfloat> orthographic =
        math::trans3d::orthographicProjection<GLfloat>({
                   {-1.f, -1.f, -1.f},
                   {2.f * math::getRatio<GLfloat>(aRenderSize), 2.f, 2.f},
               });

    setUniform(mLineProgram, "projection", orthographic);
    setUniform(mGenerativeProgram, "projection", orthographic);
 
    //
    // Line
    //
    glBindVertexArray(mLineVertexSpecification.mVertexArray);
    glUseProgram(mLineProgram);

    glDrawArrays(GL_LINE_STRIP,
                 0,
                 static_cast<GLsizei>(mLineVertices.size()));

    //
    // Generative construction
    //
    glBindVertexArray(mGenerativeVertexSpecification.mVertexArray);
    glUseProgram(mGenerativeProgram);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glDrawArraysInstanced(
        GL_TRIANGLE_STRIP,
        0,
        static_cast<GLsizei>(gGenerativeVertices.size()),
        1);
}


} // namespace curve
} // namespace ad