#pragma once

#include "gl_helpers.h"
#include "Texture.h"

#include <handy/Guard.h>

#include "GL_Loader.h"


namespace ad {

struct [[nodiscard]] FrameBuffer : public ResourceGuard<GLuint>
{
    FrameBuffer() :
        ResourceGuard<GLuint>(reserve(glGenFramebuffers),
                              [](GLuint fbId){glDeleteFramebuffers(1, &fbId);})
    {}
};

inline void bind(const FrameBuffer & aFrameBuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, aFrameBuffer);
}

inline void unbind(const FrameBuffer & aFrameBuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


// make an system to attach a render image (with depth?)
inline void attachImage(
        FrameBuffer & aFrameBuffer,
        const Texture & aTexture,
        GLenum aAttachment=GL_COLOR_ATTACHMENT0,
        GLint aTextureLayer=0)
{
    bind_guard bound{aFrameBuffer};
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, aAttachment, aTexture.mTarget, aTexture, aTextureLayer);

    // TODO completeness check should not occur after each attachment
    // maybe consolidate it behing a CompleteFrameBuffer class?
    if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error("Incomplete framebuffer (" + std::to_string(__LINE__) + ")" );
    }
}

} // namespace ad
