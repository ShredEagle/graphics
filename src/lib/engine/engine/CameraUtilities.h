#pragma once


#include <glad/glad.h>

#include <math/Rectangle.h>
#include <math/Transformations.h>


namespace ad {


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
}


} // namespace ad
