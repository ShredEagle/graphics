#include "GaussianBlur.h"

#include <renderer/Uniforms.h>


namespace ad {
namespace graphics {


// The math an logic behind the weights and linear filetering optimization are explained in:
// https://www.rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/

using Coefficients = std::array<GLfloat, 5>;
// The 12th row of the pascal triangle (minus the two smallest weights)
// (0 indexed)
static constexpr Coefficients gPascal12{924, 792, 495, 220, 66};


constexpr Coefficients computeDiscreteWeights(Coefficients aUnnormalized)
{
    GLfloat sum = aUnnormalized[0];
    for (std::size_t i = 1; i != aUnnormalized.size(); ++i)
    {
        sum += 2 * aUnnormalized[i];
    }

    for (auto & value : aUnnormalized)
    {
        value /= sum;
    }
    return aUnnormalized;
}


/// \brief Allows to group discrete weights "2-by-2", by sampling at an intermediary position
/// which is linearly filtered according to the relative weights.
///
/// It allows to leverage the linear filtering hardware of the GPU.
template <std::size_t N_size>
struct LinearFilteredKernel
{
    std::array<GLfloat, N_size> offsets;
    std::array<GLfloat, N_size> weights;
};


template <std::size_t N_size>
std::ostream & operator<<(std::ostream & aOut, const LinearFilteredKernel<N_size> & aKernel)
{
    for (std::size_t i = 0; i != N_size; ++i)
    {
        std::cout << "(o: " << aKernel.offsets[i] 
                  << ", w: " << aKernel.weights[i] << "), "
                  ;
    }
    return std::cout << "\n";
}


template <std::size_t N_initialWeights>
constexpr LinearFilteredKernel<(N_initialWeights + 2) / 2>
computeLinearFilteredWeights(const std::array<GLfloat, N_initialWeights> & aNormalized)
{
    LinearFilteredKernel<(N_initialWeights + 2) / 2> result;

    // Note: it would be possible to not make a special case of the first weight
    // by starting the loop at i = 1.
    // This would require to use half the weight though 
    // (currently, the weight[0] is not divided by 2, since it is intended to be sampled only once).
    result.offsets[0] = 0.;
    result.weights[0] = aNormalized[0];

    for (std::size_t i = 2; i < aNormalized.size(); i += 2)
    {
        result.weights[i / 2] = aNormalized[i - 1] + aNormalized[i];
        result.offsets[i / 2] = ((i - 1) * aNormalized[i - 1] + i * aNormalized[i])
                                / result.weights[i / 2];
    }

    if (aNormalized.size() % 2 == 0) // is even
    {
        result.weights.back() = aNormalized.back();
        result.offsets.back() = aNormalized.size() - 1;
    }

    return result;
}


PingPongFrameBuffers::PingPongFrameBuffers(Size2<GLsizei> aResolution, GLenum aFiltering) :
    mResolution{aResolution}
{
    for (std::size_t id = 0; id < 2; ++id)
    {
        allocateStorage(mTextures[id], GL_RGBA8, aResolution);
        // TODO handle attachment with glDrawBuffers if attaching to a color attachment > 0
        attachImage(mFrameBuffers[id], mTextures[id], GL_COLOR_ATTACHMENT0, 0);

        bind(mTextures[id]);
        // Note: Clamp to border would use a separately specified border color
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    setFiltering(aFiltering);
}


[[nodiscard]] bind_guard PingPongFrameBuffers::bindTargetFrameBuffer()
{
    return bind_guard{mFrameBuffers[mTarget]};
}


void PingPongFrameBuffers::setFiltering(GLenum aFiltering)
{
    for (std::size_t id = 0; id < 2; ++id)
    {
        bind(mTextures[id]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, aFiltering);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, aFiltering);
    }
    mFiltering = aFiltering;
}


Guard PingPongFrameBuffers::scopeFiltering(GLenum aFiltering)
{
    GLenum filtering = getFiltering();
    setFiltering(aFiltering);
    return Guard{[filtering, this]()
        {
            this->setFiltering(filtering);
        }};
}


Guard PingPongFrameBuffers::scopeViewport()
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
    constexpr Coefficients discreteWeights = computeDiscreteWeights(gPascal12);
    LinearFilteredKernel<3> linear = computeLinearFilteredWeights(discreteWeights);
    for (auto & program : mProgramSequence)
    {
        setUniformInt(program, "image", gTextureUnit); 
        setUniformFloatArray(program, "offsets", linear.offsets);
        setUniformFloatArray(program, "weights", linear.weights);
    }

    // Setup the filter directions.
    setUniform(mProgramSequence[0], "filterDirection", Vec2<GLfloat>{1.f, 0.f}); 
    setUniform(mProgramSequence[1], "filterDirection", Vec2<GLfloat>{0.f, 1.f}); 
}


void GaussianBlur::apply(int aPassCount,
                         PingPongFrameBuffers & aFrameBuffers,
                         std::optional<FrameBuffer *> aLastTarget)
{
    // Note: for the special case of a pair number of programs in the sequence,
    // there is a potential optimization allowing to avoid binding the source texture at each frame.
    // Both texture can be mapped to a different texture unit, and each program sampler in the sequence
    // uses one or the other.
    
    // If there are no pass, nothing will be copied to last target, which might indicate a logic error.
    assert(aPassCount > 0 || !aLastTarget);

    glBindVertexArray(mScreenQuad.mVertexArray); 

    // Active the texture unit that is mapped to the programs sampler.
    glActiveTexture(GL_TEXTURE0 + gTextureUnit);

    // Since the same code might be called from two different places, factorize it in a lambda
    auto executeStep = [&](int step)
    {
        glUseProgram(mProgramSequence[step % mProgramSequence.size()]);
        // binds the source texture to the texture unit (which is the texture unit for all programs).
        bind(aFrameBuffers.getTexture(Role::Source));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        aFrameBuffers.swap();
    };

    int step = 0;

    { // Scope the filtering and viewport not to affect the last target framebuffer if provided
        // When using linear filtered kernels, the linear filtering of textures must be enabled.
        auto filteringGuard = aFrameBuffers.scopeFiltering(GL_LINEAR);
        // Ensure the viewport matches the dimension of the textures for the duration of this function.
        auto viewportGuard = aFrameBuffers.scopeViewport();

        // The total number of steps is the number of passes multiplied by the number of programs to apply at each pass.
        // It will be 1 less if there is an explicit last render target.
        for (; step < aPassCount * mProgramSequence.size() - (aLastTarget ? 1 : 0); ++step)
        {
            bind_guard boundFrameBuffer = aFrameBuffers.bindTargetFrameBuffer();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            executeStep(step);
        }
    }
    // In case pass count is zero (or negative), we shoud not execute the program even once
    if (aLastTarget && aPassCount > 0)
    {
        bind_guard boundFrameBuffer{**aLastTarget};
        // No glClear on a client provided last buffer.
        executeStep(step+1);
    }
}


void GaussianBlur::drawToBoundFrameBuffer(PingPongFrameBuffers & aFrameBuffers)
{
    bind(mScreenQuad); 

    glActiveTexture(GL_TEXTURE0); // Unit 0 is assigned to the passthrough program sampler.
    bind(aFrameBuffers.getTexture(Role::Source));

    glUseProgram(mPassthrough);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


} // namespace graphics
} // namespace ad
