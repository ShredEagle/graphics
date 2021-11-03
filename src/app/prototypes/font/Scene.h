#pragma once


#include "Freetype.h"

#include "shaders.h"

#include <graphics/Timer.h>

#include <math/Vector.h>


using namespace ad::graphics;

namespace ad {
namespace font {


using Vec2 = math::Vec<2, GLfloat>;


//constexpr AttributeDescriptionList gBezierInstanceDescription{
//    {2, 2, offsetof(CubicBezier, p) + 0 * sizeof(Vec2), MappedGL<GLfloat>::enumerator},
//    {3, 2, offsetof(CubicBezier, p) + 1 * sizeof(Vec2), MappedGL<GLfloat>::enumerator},
//    {4, 2, offsetof(CubicBezier, p) + 2 * sizeof(Vec2), MappedGL<GLfloat>::enumerator},
//    {5, 2, offsetof(CubicBezier, p) + 3 * sizeof(Vec2), MappedGL<GLfloat>::enumerator},
//};


struct Scene
{
    Scene();

    void step(const Timer & aTimer);
    void render(const math::Size<2, int> aRenderSize);
};


inline Scene::Scene() /*:
    mLineProgram{makeLinkedProgram({
        {GL_VERTEX_SHADER,   gLineVertexShader},
        {GL_FRAGMENT_SHADER, gFragmentShader},
    })},
    mGenerativeProgram{makeLinkedProgram({
        {GL_VERTEX_SHADER,   gGenerativeVertexShader},
        {GL_FRAGMENT_SHADER, gFragmentShader},
    })}
    */
{
}


inline void Scene::step(const Timer & aTimer)
{
}


inline void Scene::render(const math::Size<2, int> aRenderSize)
{
}


} // namespace font
} // namespace ad
