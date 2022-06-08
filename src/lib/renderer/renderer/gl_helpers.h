#pragma once

#include "GL_Loader.h"
#include "Query.h"

#include <handy/Guard.h>

#include <math/Rectangle.h>


namespace ad {
namespace graphics {

    // Sadly, the glGen* symbols imported by the GL loader
    // are not compile time constants
//template<void(*F_glGenFun)(GLsizei, GLuint *)>
//GLint reserve()
//{
//    GLuint name;
//    (*F_glGenFun)(1, &name);
//    return name;
//}

inline GLuint reserve(void(GLAPIENTRY * aGlGenFunction)(GLsizei, GLuint *))
{
    GLuint name;
    aGlGenFunction(1, &name);
    return name;
}

class bind_guard
{
public:
    template <class T_resource, class... VT_args>
    bind_guard(const T_resource & aResource, VT_args &&... aArgs) :
        mGuard{[&aResource](){ unbind(aResource); }}
    {
        bind(aResource, std::forward<VT_args>(aArgs)...);
    }

private:
    Guard mGuard;
};


inline Guard scopeFeature(GLenum aFeature, bool aEnable)
{
    bool wasEnabled = isEnabled(aFeature);

    auto handler = [aFeature](bool enable)
    {
        if(enable) glEnable(aFeature);
        else glDisable(aFeature);
    };
    handler(aEnable);

    return Guard{ std::bind(handler, wasEnabled) };
}


inline Guard scopeDepthMask(bool aEnable)
{
    bool wasEnabled = isEnabled(GL_DEPTH_WRITEMASK);

    auto handler = [](bool enable)
    {
        glDepthMask(enable);
    };
    handler(aEnable);

    return Guard{ std::bind(handler, wasEnabled) };
}


inline Guard scopeViewport(math::Rectangle<GLint> aViewport)
{
    std::array<GLint, 4> previous;
    glGetIntegerv(GL_VIEWPORT, previous.data());

    glViewport(aViewport.xMin(), aViewport.yMin(),
               aViewport.width(), aViewport.height());

    return Guard{ [previous]()
        {
            glViewport(previous[0], previous[1], previous[2], previous[3]);
        }};
}


} // namespace graphics
} // namespace ad
