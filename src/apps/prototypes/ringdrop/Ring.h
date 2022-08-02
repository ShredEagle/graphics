#pragma once

#include <renderer/commons.h>

#include <glad/glad.h>

namespace ad {
namespace graphics {

struct Ring
{
    Vec2<GLfloat> mPosition;
    GLfloat mRotationsPerSec;
};

} // namespace graphics
} // namespace ad
