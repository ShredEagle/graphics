#pragma once


#include "commons.h"

#include <renderer/Drawing.h>

#include <math/Angle.h>
#include <math/Homogeneous.h>


namespace ad {


class TrivialShaping
{
public:
    struct Rectangle;
    struct RectangleAngle;
private:
    using InstanceData = std::vector<Rectangle>;

public:
    /// \brief Initialize the rendering to show the rectangle {{0., 0.}, aRenderResolution}.
    TrivialShaping(Size2<int> aRenderResolution);

    /// \brief Remove all shapes that were previously added.
    ///
    /// Should be called between each frame to implement animation.
    void clearShapes();

    void addRectangle(Rectangle aRectangleData);
    void addRectangle(RectangleAngle aRectangleData);

    /// \brief Render all shapes that were added since the last call to `clearShapes()`.
    void render();

    /// see: FoCG chapter 7 for this separation between several transformations.
    void setCameraTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation);
    void setProjectionTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation);

private:
    DrawContext mDrawContext;
    InstanceData mInstances;
};


struct TrivialShaping::Rectangle
{
    ad::Rectangle<GLfloat> mGeometry;
    math::Matrix<3, 3, GLfloat> mMatrixTransform;
    Color mColor;
};

struct TrivialShaping::RectangleAngle
{
    ad::Rectangle<GLfloat> mGeometry;
    Color mColor;
    math::Radian<GLfloat> angle{0.};
    math::Position<2, GLfloat> center = mGeometry.origin();
};

} // namespace ad
