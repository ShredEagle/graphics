#include "GaussianBlur.h"

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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
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


const Texture & GaussianBlur::apply(int aPassCount, PingPongFrameBuffers & aFrameBuffers)
{
    glBindVertexArray(mScreenQuad.mVertexArray); 

    glActiveTexture(GL_TEXTURE0);
    bind(aFrameBuffers.getTexture(Role::Source));
    glActiveTexture(GL_TEXTURE1);
    bind(aFrameBuffers.getTexture(Role::Target));

    for (int step = 0; step != aPassCount * mProgramSequence.size(); ++step)
    {
        glUseProgram(mProgramSequence[step % mProgramSequence.size()]);
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
