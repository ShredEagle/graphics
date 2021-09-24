#pragma once


#include "shaders.h"

#include <engine/Timer.h>

#include <renderer/Drawing.h>
#include <renderer/Uniforms.h>

#include <math/Transformations.h>
#include <math/Vector.h>
#include <math/VectorUtilities.h>


// The approach is to draw miter joints with the minimal number of vertices.
// A geometry shader is used to find the miter vector, 
// then place the corner vertices at the correct distance along miter vector.
// see: https://en.sfml-dev.org/forums/index.php?topic=21620.msg153681#msg153681
// It is compared to the naive approach with overdraw and discontinuities.


namespace ad {


struct Vertex
{
    Vec2<GLfloat> position;
};


const std::vector<Vertex> gVertices{
    {{-1.0f, -0.4f}}, // Extend the first segment for adjacency
    {{-0.8f, -0.4f}},
    {{-0.5f, -0.4f}},
    {{ 0.5f,  0.4f}},
    {{ 0.8f,  0.4f}},
    {{ 1.0f,  0.4f}}, // Extend the last segment for adjacency
};


constexpr AttributeDescriptionList gVertexDescription{
    {0, 2, offsetof(Vertex, position), MappedGL<GLfloat>::enumerator},
};


struct Scene
{
    Scene();

    void step(const Timer & aTimer);
    void render(const Size2<int> aRenderSize);

    VertexSpecification mVertexSpecification;
    Program  mProgramNaive;
    Program  mProgramMiter;
};


inline Scene::Scene() :
    mVertexSpecification{},
    mProgramNaive{makeLinkedProgram({
              {GL_VERTEX_SHADER,   gVertexShader},
              {GL_GEOMETRY_SHADER, gNaiveGeometryShader},
              {GL_FRAGMENT_SHADER, gFragmentShader},
    })},
    mProgramMiter{makeLinkedProgram({
              {GL_VERTEX_SHADER,   gVertexShader},
              {GL_GEOMETRY_SHADER, gMiterGeometryShader},
              {GL_FRAGMENT_SHADER, gFragmentShader},
    })}
{
    appendToVertexSpecification<const Vertex>(mVertexSpecification,
                                              gVertexDescription,
                                              gVertices);
}


inline void Scene::step(const Timer & aTimer)
{
    setUniformFloat(mProgramNaive, "lineHalfWidth", 0.05f + 0.04f * std::cos(aTimer.time()));
    setUniformFloat(mProgramMiter, "lineHalfWidth", 0.05f + 0.04f * std::cos(aTimer.time()));
}



inline void Scene::render(const Size2<int> aRenderSize)
{
    glBindVertexArray(mVertexSpecification.mVertexArray);

    math::AffineMatrix<4, GLfloat> orthographic =
        math::trans3d::orthographicProjection<GLfloat>({
                   {-1.f, -1.f, 1.f},
                   {2.f * math::getRatio<GLfloat>(aRenderSize), 2.f, 2.f},
               });

    //
    // Naive GS
    //
    glUseProgram(mProgramNaive);

    setUniform(mProgramNaive, "projection", orthographic);
    setUniform(mProgramNaive, "model", math::trans3d::translate<GLfloat>({0.f, 0.45f, 0.f}));
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glDrawArrays(GL_LINE_STRIP,
                 // The range discard the two extensions for adjacency
                 1,
                 static_cast<GLsizei>(gVertices.size()) - 2);

    setUniform(mProgramNaive, "model", math::trans3d::translate<GLfloat>({0.f, 0.2f, 0.f}));
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glDrawArrays(GL_LINE_STRIP,
                 1,
                 static_cast<GLsizei>(gVertices.size()) - 2);

    //
    // Mitter GS
    //
    glUseProgram(mProgramMiter);

    setUniform(mProgramMiter, "projection", orthographic);
    setUniform(mProgramMiter, "model", math::trans3d::translate<GLfloat>({0.f, -0.2f, 0.f}));

    glDrawArrays(GL_LINE_STRIP_ADJACENCY,
                 0,
                 static_cast<GLsizei>(gVertices.size()));

    setUniform(mProgramMiter, "model", math::trans3d::translate<GLfloat>({0.f, -0.45f, 0.f}));
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glDrawArrays(GL_LINE_STRIP_ADJACENCY,
                 0,
                 static_cast<GLsizei>(gVertices.size()));
}


} // namespace ad