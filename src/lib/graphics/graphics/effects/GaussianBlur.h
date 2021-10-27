#pragma once


#include "GaussianBlur-shaders.h"

#include "../commons.h"
#include "../shaders.h"

#include "../detail/ScreenQuad.h"

#include <renderer/commons.h>
#include <renderer/FrameBuffer.h>
#include <renderer/gl_helpers.h>
#include <renderer/Shading.h>
#include <renderer/Texture.h>

#include <array>


namespace ad {
namespace graphics {


enum class Role
{
    Target = 0,
    Source = 1,
};

// TODO Handle window (default framebuffer) resizing, possibly by listening.
struct PingPongFrameBuffers
{
    PingPongFrameBuffers(Size2<GLsizei> aResolution);

    /// \brief Bind the current target framebuffer, so draw operations will affect it.
    /// \return A guard to unbind (i.e. to restore default framebuffer).
    [[nodiscard]] bind_guard bindTargetFrameBuffer();

    /// \brief Set the viewport to match the frame buffers resolution, and return a Guard
    /// to restore previous viewport.
    Guard setupViewport();

    /// \brief Swap "Target" and "Source" Framebuffers and Textures.
    void swap();

    const Texture & getTexture(Role aRole);

    Size2<GLsizei> mResolution;
    std::array<FrameBuffer, 2> mFrameBuffers;
    std::array<Texture, 2> mTextures{GL_TEXTURE_2D, GL_TEXTURE_2D};
    std::size_t mTarget{1};
};


class GaussianBlur
{
public:
    GaussianBlur();

    const Texture & apply(int aPassCount, PingPongFrameBuffers & aFrameBuffers);

    // TODO relocate somewhere general
    /// \brief Notably usefull if the default framebuffer is currently active;
    void drawToBoundFrameBuffer(PingPongFrameBuffers & aFrameBuffers);

private:
    // TODO use a global screen quad instead
    const VertexSpecification mScreenQuad{detail::make_ScreenQuad()};

    std::array<Program, 2> mProgramSequence{
        makeLinkedProgram({
            {GL_VERTEX_SHADER, gPassthroughVertexShader},
            {GL_FRAGMENT_SHADER, gaussianblur::gHorizontalBlurFragmentShader}
        }),
        makeLinkedProgram({
            {GL_VERTEX_SHADER, gPassthroughVertexShader},
            {GL_FRAGMENT_SHADER, gaussianblur::gVerticalBlurFragmentShader}
        })
    };

    static constexpr GLsizei gTextureUnit = 1;
    Program mPassthrough{detail::make_PassthroughProgram()};
};


} // namespace graphics
} // namespace ad
