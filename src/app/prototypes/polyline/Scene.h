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
    {{-1.0f, -0.5f}}, // Extend the first segment for adjacency
    {{-0.8f, -0.5f}},
    {{-0.5f, -0.5f}},
    {{ 0.5f,  0.5f}},
    {{ 0.8f,  0.5f}},
    {{ 1.0f,  0.5f}}, // Extend the last segment for adjacency
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
              {GL_GEOMETRY_SHADER, gMiterGeometryShader},
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

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glDrawArrays(//GL_LINE_STRIP,
                 GL_LINE_STRIP_ADJACENCY,
                 0,
                 static_cast<GLsizei>(gVertices.size()));
}

} // namespace ad