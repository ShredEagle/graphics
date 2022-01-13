#pragma once


#include <glad/glad.h>

#include <math/Rectangle.h>
#include <math/Transformations.h>
#include <math/VectorUtilities.h>


namespace ad {
namespace graphics {


/// \brief Setup the 2D engine `aEngine` camera to view world coordinates in `aViewedRectangle`.
/// \note The projection transformation part maps to OpenGL NDC [-1, 1]^2
template <class T_engine2D>
void setViewedRectangle(T_engine2D & aEngine, math::Rectangle<GLfloat> aViewedRectangle)
{
    aEngine.setCameraTransformation(math::trans2d::translate(- aViewedRectangle.center().as<math::Vec>()));
    aEngine.setProjectionTransformation(
        math::trans2d::orthographicProjection<GLfloat>(math::Rectangle<GLfloat>{
            math::Position<2, GLfloat>::Zero(),
            aViewedRectangle.dimension()
        }.centered()));
}


enum class ViewOrigin
{
    Unchanged,
    LowerLeft // The window's lower left corner is aligned with world's origin (0, 0)
};

/// \brief Set the size of the viewport in sprite pixels (assuming the default pixel world size of 1).
///
/// This is a helper around `setProjectionTransformation()`: it is setting the world size of the viewport. 
/// It makes it convenient to work with the virtual pixels as world unit.
///
/// \important the viewport will be [-width/2, -height/2] x [width/2, height/2], unless aOrigin is LowerLeft,
/// then the viewport will be [0, 0] x [width, height].
template <class T_engine2D>
void setViewportVirtualResolution(T_engine2D & aEngine,
                                  math::Size<2, int> aViewportPixelSize,
                                  ViewOrigin aOrigin = ViewOrigin::Unchanged)
{
    math::Rectangle<GLfloat> viewportArea{
        math::Position<2, GLfloat>::Zero(),
        static_cast<math::Size<2, GLfloat>>(aViewportPixelSize)
    };

    aEngine.setProjectionTransformation(
        math::trans2d::orthographicProjection<GLfloat>(viewportArea.centered()));

    if (aOrigin == ViewOrigin::LowerLeft)
    {
        aEngine.setCameraTransformation(math::trans2d::translate(-viewportArea.center().as<math::Vec>()));
    }
}


/// \param aCameraPosition_w is the camera position in world frame.
/// \param aViewVolume_c is the view volume in the camera frame. see `getViewVolume()`.
template <class T_engine3D>
void setOrthographicView(T_engine3D & aEngine,
                         math::Position<3, GLfloat> aCameraPosition_w,
                         math::Box<GLfloat> aViewVolume_c)
{
    aEngine.setCameraTransformation(math::trans3d::translate(-aCameraPosition_w.as<math::Vec>()));
    aEngine.setProjectionTransformation(math::trans3d::orthographicProjection<GLfloat>(aViewVolume_c));
}


inline math::AffineMatrix<4> getCameraTransform(math::Position<3> aCameraPosition,
                                                math::Vec<3> aGazeDirection,
                                                math::Vec<3> aUpDirection = {0., 1., 0.})
{
    math::Frame<3> cameraFrame{
        aCameraPosition,
        math::OrthonormalBase<3>::MakeFromTwoVectors(-aGazeDirection, aUpDirection)
    };

    return math::trans3d::canonicalToFrame(cameraFrame);
}


/// \brief Return the view rectangle centered on zero with height aBufferHeight, and
/// a proportionnal width based on the render resolution ratio.
inline math::Rectangle<GLfloat> getViewRectangle(math::Size<2, int> aRenderResolution,
                                                 GLfloat aBufferHeight)
{
    math::Size<2, GLfloat> size{
        math::makeSizeFromHeight(aBufferHeight, math::getRatio<GLfloat>(aRenderResolution))
    };

    return math::Rectangle<GLfloat>{
        math::Position<2, GLfloat>{
            -size.width() / 2.f,
            -size.height() / 2.f},
        size,
    };
}


inline math::Box<GLfloat> getViewVolume(math::Size<2, int> aRenderResolution,
                                 GLfloat aBufferHeight,   
                                 GLfloat aNearPlaneZ,
                                 GLfloat aDepth)
{
    math::Size<3, GLfloat> size{
        math::makeSizeFromHeight(aBufferHeight, math::getRatio<GLfloat>(aRenderResolution)),
        aDepth
    };

    return math::Box<GLfloat>{
        math::Position<3, GLfloat>{
            -size.width() / 2.f,
            -size.height() / 2.f,
            aNearPlaneZ},
        size,
    };
}


} // namespace graphics
} // namespace ad
