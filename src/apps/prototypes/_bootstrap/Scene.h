#pragma once


#include "shaders.h"

#include <engine/Timer.h>
#include <renderer/Drawing.h>

#include <math/Vector.h>


namespace ad {


struct Vertex
{
    Vec2<GLfloat> position;
};


const std::vector<Vertex> gVertices{
    {{-0.8f, -0.8f}},
    {{0.8f, 0.8f}},
};


constexpr AttributeDescriptionList gVertexDescription{
    {0, 2, offsetof(Vertex, position), MappedGL<GLfloat>::enumerator},
};


struct Scene
{
    Scene();

    void step(const Timer & aTimer);
    void render();

    VertexSpecification mVertexSpecification;
    Program  mProgram;
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
                                              gVertices);
}


inline void Scene::step(const Timer & /*aTimer*/)
{}


inline void Scene::render()
{
    glBindVertexArray(mVertexSpecification.mVertexArray);
    glUseProgram(mProgram);

    glDrawArrays(GL_LINE_STRIP,
                 0,
                 static_cast<GLsizei>(gVertices.size()));
}

} // namespace ad