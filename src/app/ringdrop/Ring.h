#pragma once

#include <math/Vector.h>

#include <glad/glad.h>

namespace ad {

struct Ring
{
    math::Vec2<GLfloat> mPosition;
    GLfloat mRotationsPerSec;
};

} // namespace ad
