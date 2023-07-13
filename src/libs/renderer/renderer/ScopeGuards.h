#pragma once


#include "SynchronousQueries.h"

#include <handy/Guard.h>

#include <math/Rectangle.h>


namespace ad {
namespace graphics {


// Note: The naming convention here is that "scoping" means changing the value,
// then returning it to the previous value when the scope is exited.

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


inline Guard scopeCullFace(GLenum aMode)
{
    GLenum previousMode = getEnum(GL_CULL_FACE_MODE);

    auto handler = [](GLenum mode)
    {
        glCullFace(mode);
    };
    handler(aMode);

    return Guard{ std::bind(handler, previousMode) };
}


// Note: The separate control for FRONT and BACK faces has been deprecated (at least in 4.6)
inline Guard scopePolygonMode(GLenum aMode)
{
    GLenum previousMode = getEnum(GL_POLYGON_MODE);

    auto handler = [](GLenum mode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, mode);
    };
    handler(aMode);

    return Guard{ std::bind(handler, previousMode) };
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


template <class T_index>
inline Guard scopePrimitiveRestartIndex(T_index aIndex)
{
    bool wasEnabled = isEnabled(GL_PRIMITIVE_RESTART);
    // We want to restore the restart index value even if it is not enabled.
    GLint previousIndex;
    glGetIntegerv(GL_PRIMITIVE_RESTART_INDEX, &previousIndex);

    glPrimitiveRestartIndex(aIndex);

    if(wasEnabled)
    {
        return Guard{ [previousIndex]()
            {
                glPrimitiveRestartIndex(previousIndex); 
            }};
    }
    else
    {
        glEnable(GL_PRIMITIVE_RESTART);
        return Guard{ [previousIndex]()
            {
                glDisable(GL_PRIMITIVE_RESTART);
                glPrimitiveRestartIndex(previousIndex);
            }};
    }
}


} // namespace graphics
} // namespace ad