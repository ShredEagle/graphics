#pragma once

#include <renderer/commons.h>

#include <glad/glad.h>

namespace ad {

struct Ring
{
    Vec2<GLfloat> mPosition;
    GLfloat mRotationsPerSec;
};

} // namespace ad
