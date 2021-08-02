#pragma once


#include "commons.h"

#include <renderer/Drawing.h>


namespace ad {


class TrivialShaping
{
public:
    struct Rectangle;
private:
    using InstanceData = std::vector<Rectangle>;

public:
    TrivialShaping(Size2<int> aRenderResolution);

    /// \brief Remove all shapes that were previously added.
    ///
    /// Should be called between each frame to implement animation.
    void clearShapes();

    void addRectangle(Rectangle aRectangleData);

    /// \brief Render all shapes that were added since the last call to `clearShapes()`.
    void render();

private:
    void setBufferResolution(Size2<int> aNewResolution);

private:
    DrawContext mDrawContext;
    InstanceData mInstances;
};


struct TrivialShaping::Rectangle
{
    ad::Rectangle<GLint> mGeometry;
    Color mColor;
};

} // namespace ad
