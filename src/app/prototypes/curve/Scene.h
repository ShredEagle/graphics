#pragma once


#include "shaders.h"

#include <engine/Timer.h>

#include <renderer/Drawing.h>
#include <renderer/Uniforms.h>

#include <math/Transformations.h>
#include <math/Vector.h>
#include <math/VectorUtilities.h>


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


struct Vertex
{
    Vec2 position;
};


constexpr AttributeDescriptionList gVertexDescription{
    {0, 2, offsetof(Vertex, position), MappedGL<GLfloat>::enumerator},
};


struct Scene
{
    Scene();

    void step(const Timer & aTimer);
    void render(const Size2<int> aRenderSize);

    constexpr static int gSubdivisions = 20;
    
    VertexSpecification mVertexSpecification;
    Program  mProgram;

    std::vector<Vertex> mVertices;
    CubicBezier mBezier{ {Vec2{-0.8f, -0.5f}, Vec2{-0.3f, 1.8f}, Vec2{0.3f, -1.8f}, Vec2{0.8f, 0.5f}} };
};


inline Scene::Scene() :
    mVertexSpecification{},
    mProgram{makeLinkedProgram({
              {GL_VERTEX_SHADER,   gVertexShader},
              {GL_FRAGMENT_SHADER, gFragmentShader},
    })}
{
    appendToVertexSpecification<const Vertex>(mVertexSpecification,
                                              gVertexDescription,
                                              {});

    for (std::size_t i = 0; i <= gSubdivisions; ++i)
    {
        mVertices.push_back(Vertex{deCasteljau((GLfloat)i/gSubdivisions, mBezier)});
    }

    respecifyBuffer<Vertex>(mVertexSpecification.mVertexBuffers[0], mVertices);
}


inline void Scene::step(const Timer & aTimer)
{
    //setUniformFloat(mProgram, "lineHalfWidth", 0.05f + 0.04f * std::cos(aTimer.time()));
}


inline void Scene::render(const Size2<int> aRenderSize)
{
    glBindVertexArray(mVertexSpecification.mVertexArray);
    glUseProgram(mProgram);

    math::AffineMatrix<4, GLfloat> orthographic =
        math::trans3d::orthographicProjection<GLfloat>({
                   {-1.f, -1.f, 1.f},
                   {2.f * math::getRatio<GLfloat>(aRenderSize), 2.f, 2.f},
               });


    glDrawArrays(GL_LINE_STRIP,
                 0,
                 static_cast<GLsizei>(mVertices.size()));
}


} // namespace curve
} // namespace ad