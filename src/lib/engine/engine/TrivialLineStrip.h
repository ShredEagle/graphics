#pragma once


#include "commons.h"

#include <renderer/Drawing.h>

#include <math/Homogeneous.h>


namespace ad {


class TrivialLineStrip
{
public:
    struct LinePoint;

    /// \brief Initialize the rendering to show the rectangle {{0., 0.}, aRenderResolution}.
    TrivialLineStrip(Size2<int> aRenderResolution);

    /// \brief Remove all shapes that were previously added.
    ///
    /// Should be called between each frame to implement animation.
    void clearLines();

    void addLine(std::initializer_list<LinePoint> aPoints);

    void outlineRectangle(const Rectangle<GLfloat> & aRectangle, const Color aColor);

    /// \brief Render all shapes that were added since the last call to `clearShapes()`.
    void render();

    /// see: FoCG chapter 7 for this separation between several transformations.
    void setCameraTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation);
    void setProjectionTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation);

private:
    using Index = GLuint;

    static constexpr Index gRestartIndex = std::numeric_limits<Index>::max();

    DrawContext mDrawContext;
    std::vector<LinePoint> mVertexAttributes;
    IndexBufferObject mIbo;
    Index mNextIndex;
    std::vector<Index> mIndices;
};


struct TrivialLineStrip::LinePoint
{
    math::Position<2, GLfloat> mPosition;
    Color mColor;
};

} // namespace ad
