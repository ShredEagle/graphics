#pragma once


#include "GaussianBlur-shaders.h"

#include "../commons.h"
#include "../shaders.h"

#include "../detail/UnitQuad.h"

#include <renderer/commons.h>
#include <renderer/FrameBuffer.h>
#include <renderer/gl_helpers.h>
#include <renderer/Shading.h>
#include <renderer/Texture.h>

#include <array>
#include <optional>


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
    explicit PingPongFrameBuffers(Size2<GLsizei> aResolution, GLenum aFiltering = GL_NEAREST);


    /// \brief High level function, to prepare everything for client rendering to current target,
    /// and then appropriate clean-up and buffer swapping on guard destruction.
    // Note: hard to implement, because it is not possible to just capture the move-only guards in the returned
    // Guard lambda (the lambda would be move only, so it could not be used to construct the std::function in Guard)
    //Guard scopeForClientDrawing();

    /// \brief Bind the current target framebuffer, so draw operations will affect it.
    /// \return A guard to unbind (i.e. to restore default framebuffer).
    [[nodiscard]] bind_guard bindTargetFrameBuffer();

    /// \brief Set the viewport to match the frame buffers resolution, and return a Guard
    /// to restore previous viewport.
    Guard scopeViewport();

    void setFiltering(GLenum aFiltering);
    GLenum getFiltering()
    { return mFiltering; }

    /// \attention This instance must outlive the returned guard.
    Guard scopeFiltering(GLenum aFiltering);

    /// \brief Swap "Target" and "Source" Framebuffers and Textures.
    void swap();

    const Texture & getTexture(Role aRole);

    GLenum mFiltering;
    Size2<GLsizei> mResolution;
    std::array<FrameBuffer, 2> mFrameBuffers;
    std::array<Texture, 2> mTextures{GL_TEXTURE_2D, GL_TEXTURE_2D};
    std::size_t mTarget{1};
};


// Anecdote: I compared on 2021/10/27 the result of discrete 5 weights filtering,
// and the mathematically equivalent linear filtered 3 weights + offsets filtering.
// The result were visually indistinguishable.
// (Which was not the case when using the 3 weight with nearest filtering.)
class GaussianBlur
{
public:
    GaussianBlur();

    /// \param aLastTarget A framebuffer that can be explicilty provided in order to become
    /// the render target for the last drawing step.
    /// This can notably spare a copy from the PingPongFrameBuffer to the default framebuffer.
    ///
    /// \important When an explicit last target framebuffer is provided, it will be drawn to
    /// with the viewport and filtering parameters that are active when invoking the function.
    /// It will not be cleared either.
    void apply(int aPassCount, 
               PingPongFrameBuffers & aFrameBuffers,
               std::optional<FrameBuffer *> aLastTarget = std::nullopt);

    // TODO relocate somewhere general
    /// \brief Notably usefull if the default framebuffer is currently active.
    void drawToBoundFrameBuffer(PingPongFrameBuffers & aFrameBuffers);

private:
    // TODO use a global screen quad instead
    const VertexSpecification mScreenQuad{detail::make_UnitQuad()};

    // Note: Same program sources, but the uniform values will differ.
    std::array<Program, 2> mProgramSequence{
        makeLinkedProgram({
            {GL_VERTEX_SHADER, gPassthroughVertexShader},
            {GL_FRAGMENT_SHADER, gaussianblur::g1DLinearFilterFragmentShader}
        }),
        makeLinkedProgram({
            {GL_VERTEX_SHADER, gPassthroughVertexShader},
            {GL_FRAGMENT_SHADER, gaussianblur::g1DLinearFilterFragmentShader}
        })
    };

    static constexpr GLsizei gTextureUnit = 1;
    Program mPassthrough{detail::make_PassthroughProgram()};
};


} // namespace graphics
} // namespace ad
