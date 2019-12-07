#pragma once

#include "gl_helpers.h"

#include <handy/Guard.h>

#include <glad/glad.h>


namespace ad {

struct [[nodiscard]] FrameBuffer : public ResourceGuard<GLuint>
{
    FrameBuffer() :
        ResourceGuard<GLuint>(reserve(glGenFramebuffers),
                              [](GLuint fbId){glDeleteFramebuffers(1, &fbId);})
    {}
};

} // namespace ad
