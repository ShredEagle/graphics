#pragma once


#include <math/Rectangle.h>
#include <math/Transformations.h>


namespace ad {


/// \brief Setup the 2D engine `aEngine` camera to view world coordinates in `aViewedRectangle`.
/// \note The projection transformation part maps to OpenGL NDC [-1, 1]^2
template <class T_engine2D>
void setViewedRectangle(T_engine2D & aEngine, math::Rectangle<GLfloat> aViewedRectangle)
{
    aEngine.setCameraTransformation(math::trans2d::translate(- aViewedRectangle.bottomLeft().as<math::Vec>()));
    aEngine.setProjectionTransformation(math::trans2d::window<GLfloat>(
        { {  0.f,  0.f }, aViewedRectangle.dimension() },
        { { -1.f, -1.f }, { 2.f, 2.f } }
    ));
}


} // namespace ad
