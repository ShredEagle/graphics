#pragma once

#include "gl_helpers.h"
#include "ScopeGuards.h"
#include "Texture.h"

#include <handy/Guard.h>


namespace ad {
namespace graphics {

struct [[nodiscard]] FrameBuffer : public ResourceGuard<const GLuint>
{
    FrameBuffer() :
        ResourceGuard<const GLuint>(reserve(glGenFramebuffers),
                              [](GLuint fbId){glDeleteFramebuffers(1, &fbId);})
    {}

    static FrameBuffer & Default()
    {
        static FrameBuffer instance{DefaultTag{}};
        return instance;
    }

private:
    struct DefaultTag {};
    FrameBuffer(DefaultTag) :
        ResourceGuard<const GLuint>{0, [](GLuint){}}
    {}
};


enum class FrameBufferTarget
{
    Draw = GL_DRAW_FRAMEBUFFER,
    Read = GL_READ_FRAMEBUFFER,
};


/// \brief Bind the framebuffer to both Read and Draw targets.
inline void bind(const FrameBuffer & aFrameBuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, aFrameBuffer);
}


inline void bind(Name<FrameBuffer> aFrameBuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, aFrameBuffer);
}


/// \brief Reset to default framebuffer for both Read and Draw targets.
inline void unbind(const FrameBuffer & /*aFrameBuffer*/)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


inline void bind(const FrameBuffer & aFrameBuffer, FrameBufferTarget aTarget)
{
    glBindFramebuffer(static_cast<GLenum>(aTarget), aFrameBuffer);
}


inline void bind(Name<FrameBuffer> aFrameBuffer, FrameBufferTarget aTarget)
{
    glBindFramebuffer(static_cast<GLenum>(aTarget), aFrameBuffer);
}


inline void unbind(const FrameBuffer &, FrameBufferTarget aTarget)
{
    glBindFramebuffer(static_cast<GLenum>(aTarget), 0);
}


inline Name<FrameBuffer> getBound(const FrameBuffer &, FrameBufferTarget aTarget)
{
    GLint current;
    glGetIntegerv(getGLMappedBufferBinding(static_cast<GLenum>(aTarget)), &current);
    return Name<FrameBuffer>{(GLuint)current, Name<FrameBuffer>::UnsafeTag{}};
}


/// \brief Specialization for the situation where both Read and Write targets are bound.
template <>
inline ScopedBind::ScopedBind(const FrameBuffer & aResource) :
    mGuard{[previousRead = getBound(aResource, FrameBufferTarget::Read),
            previousDraw = getBound(aResource, FrameBufferTarget::Draw)]()
           {
               glBindFramebuffer(GL_DRAW_FRAMEBUFFER, previousDraw);
               glBindFramebuffer(GL_READ_FRAMEBUFFER, previousRead);
           }}
{
    bind(aResource);
}


// make a system to attach a render image (with depth?)
inline void attachImage(
        FrameBuffer & aFrameBuffer,
        const Texture & aTexture,
        GLenum aAttachment=GL_COLOR_ATTACHMENT0,
        GLint aTextureLayer=0)
{
    ScopedBind bound{aFrameBuffer};
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, aAttachment, aTexture.mTarget, aTexture, aTextureLayer);

    // TODO completeness check should not occur after each attachment
    // maybe consolidate it behing a CompleteFrameBuffer class?
    if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error("Incomplete framebuffer (" + std::to_string(__LINE__) + ")" );
    }
}


} // namespace graphics
} // namespace ad
