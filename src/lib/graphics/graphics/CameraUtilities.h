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
        math::trans2d::orthographicProjection<GLfloat>({
            -(aViewedRectangle.dimension() / 2.).as<math::Position>(),
            aViewedRectangle.dimension() }));
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


math::Rectangle<GLfloat> getViewRectangle(math::Size<2, int> aRenderResolution,
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


math::Box<GLfloat> getViewVolume(math::Size<2, int> aRenderResolution,
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
