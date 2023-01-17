#pragma once


#include "Query.h"

#include <handy/Guard.h>

#include <math/Rectangle.h>


namespace ad {
namespace graphics {


// TODO might become a single template function.
class ScopedBind
{
public:
    template <class T_resource, class... VT_args>
    ScopedBind(const T_resource & aResource, VT_args &&... aArgs) :
        mGuard{[previous = getBound(aResource, aArgs...), aArgs...]()
               { bind(previous, aArgs...); }}
    {
        bind(aResource, aArgs...);
    }

    // TODO scoped bind for Indexed version

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