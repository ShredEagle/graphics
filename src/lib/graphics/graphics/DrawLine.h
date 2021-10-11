#pragma once


#include "commons.h"

#include <renderer/Drawing.h>


namespace ad {


class DrawLine
{
public:
    struct Line;
private:
    using InstanceData = std::vector<Line>;

public:
    DrawLine(Size2<int> aRenderResolution);

    /// \brief Remove all shapes that were previously added.
    ///
    /// Should be called between each frame to implement animation.
    void clearShapes();

    void addLine(Line aLine);

    /// \brief Render all shapes that were added since the last call to `clearShapes()`.
    void render();

private:
    void setBufferResolution(Size2<int> aNewResolution);

private:
    DrawContext mDrawContext;
    InstanceData mInstances;
};


struct DrawLine::Line
{
    ad::Position2<GLint> mOrigin;
    ad::Position2<GLint> mEnd;
    GLfloat width;
    Color mColor;
};

} // namespace ad
