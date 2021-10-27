#include "GaussianBlur.h"

#include <renderer/Uniforms.h>


namespace ad {
namespace graphics {


PingPongFrameBuffers::PingPongFrameBuffers(Size2<GLsizei> aResolution) :
    mResolution{aResolution}
{
    for (std::size_t id = 0; id < 2; ++id)
    {
        allocateStorage(mTextures[id], GL_RGBA8, aResolution);
        // TODO handle attachment with glDrawBuffers if attaching to a color attachment > 0
        attachImage(mFrameBuffers[id], mTextures[id], GL_COLOR_ATTACHMENT0, 0);

        bind(mTextures[id]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // Note: Clamp to border would use a separately specified border color
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}


[[nodiscard]] bind_guard PingPongFrameBuffers::bindTargetFrameBuffer()
{
    return bind_guard{mFrameBuffers[mTarget]};
}


Guard PingPongFrameBuffers::setupViewport()
{
    // save viewport parameters, to restore it with the guard.
    GLint backup[4];
    glGetIntegerv(GL_VIEWPORT, backup);

    // set the current viewport to match the framebuffers in this.
    glViewport(0, 0, mResolution.width(), mResolution.height());

    return Guard{[backup]()
        {
            glViewport(backup[0], backup[1], (GLsizei)backup[2], (GLsizei)backup[3]); 
        }};
}


void PingPongFrameBuffers::swap()
{
    mTarget = (mTarget + 1) % 2;
}


const Texture & PingPongFrameBuffers::getTexture(Role aRole)
{
    return mTextures[(mTarget + static_cast<int>(aRole)) % 2];
}


GaussianBlur::GaussianBlur()
{
    for (auto & program : mProgramSequence)
    {
        setUniformInt(program, "image", gTextureUnit); 
    }
}


const Texture & GaussianBlur::apply(int aPassCount, PingPongFrameBuffers & aFrameBuffers)
{
    // Note: for the special case of a pair number of programs in the sequence,
    // there is a potential optimization allowing to avoid binding the source texture at each frame.
    // Both texture can be mapped to a different texture unit, and each program sampler in the sequence
    // uses one or the other.

    glBindVertexArray(mScreenQuad.mVertexArray); 

    // Active the texture unit that is mapped to the programs sampler.
    glActiveTexture(GL_TEXTURE0 + gTextureUnit);

    // Ensure the viewport matches the dimension of the textures for the duration of this function.
    auto viewportGuard = aFrameBuffers.setupViewport();

    // The total number of steps is the number of passes multiplied by the number of programs to apply at each pass.
    for (int step = 0; step != aPassCount * mProgramSequence.size(); ++step)
    {
        glUseProgram(mProgramSequence[step % mProgramSequence.size()]);
        // binds the source texture to the texture unit (which is the texture unit for all programs).
        bind(aFrameBuffers.getTexture(Role::Source));
        bind_guard boundFrameBuffer = aFrameBuffers.bindTargetFrameBuffer();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        aFrameBuffers.swap();
    }

    return aFrameBuffers.getTexture(Role::Source);
}


void GaussianBlur::drawToBoundFrameBuffer(PingPongFrameBuffers & aFrameBuffers)
{
    bind(mScreenQuad); 

    glActiveTexture(GL_TEXTURE0);
    bind(aFrameBuffers.getTexture(Role::Source));

    glUseProgram(mPassthrough);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


} // namespace graphics
} // namespace ad
