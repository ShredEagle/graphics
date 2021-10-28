#pragma once


#include "commons.h"

#include <renderer/Drawing.h>


namespace ad {
namespace graphics {


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
    void render() const;

private:
    void setBufferResolution(Size2<int> aNewResolution);

private:
    DrawContext mDrawContext;
    InstanceData mInstances;
};


struct DrawLine::Line
{
    Position2<GLint> mOrigin;
    Position2<GLint> mEnd;
    GLfloat width;
    Color mColor;
};

} // namespace graphics
} // namespace ad
