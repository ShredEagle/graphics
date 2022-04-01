#pragma once

#include <handy/Guard.h>

#include "GL_Loader.h"
#include "Query.h"

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


inline Guard scopeDepthTest(bool aEnable)
{
    bool wasEnabled = isEnabled(GL_DEPTH_TEST);

    auto handler = [](bool enable)
    {
        if(enable) glEnable(GL_DEPTH_TEST);
        else glDisable(GL_DEPTH_TEST);
    };
    handler(aEnable);

    return Guard{ std::bind(handler, wasEnabled) };
}


} // namespace graphics
} // namespace ad
