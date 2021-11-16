#pragma once


#include "commons.h"

#include <renderer/Drawing.h>

#include <math/Angle.h>
#include <math/Homogeneous.h>


namespace ad {
namespace graphics {


class TrivialShaping
{
public:
    struct Rectangle;
    struct RectangleAngle;

public:
    /// \brief Initialize the rendering to show the rectangle {{0., 0.}, aRenderResolution}.
    TrivialShaping(Size2<int> aRenderResolution);

    /// \brief Replace all instances that will be drawn when calling `render()`.
    ///
    /// Actually push to the vertex buffers.
    void updateInstances(gsl::span<const Rectangle> aInstances);

    /// \brief Render all shapes that were added since the last call to `clearShapes()`.
    void render() const;

    /// see: FoCG chapter 7 for this separation between several transformations.
    void setCameraTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation);
    void setProjectionTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation);

private:
    DrawContext mDrawContext;
    GLsizei mInstanceCount{0};
};


struct TrivialShaping::Rectangle
{
    math::Rectangle<GLfloat> mGeometry;
    Color mColor;
    math::Matrix<3, 3, GLfloat> mMatrixTransform = math::Matrix<3, 3, GLfloat>::Identity();
};

struct TrivialShaping::RectangleAngle
{
    math::Rectangle<GLfloat> mGeometry;
    math::Radian<GLfloat> angle;
    Color mColor;
    math::Position<2, GLfloat> center = mGeometry.origin();

    /*implicit*/ operator Rectangle ();
};


} // namespace graphics
} // namespace ad
