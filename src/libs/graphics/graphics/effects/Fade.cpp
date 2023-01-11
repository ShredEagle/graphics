#include "Fade.h"

#include "Fade-shaders.h"
#include "../shaders.h"

#include "../detail/UnitQuad.h"

#include <handy/Guard.h>
#include <renderer/Drawing.h>
#include <renderer/FrameBuffer.h>
#include <renderer/Texture.h>
#include <renderer/Uniforms.h>


namespace ad {
namespace graphics {


Fade::Fade() :
    mUnitQuad{detail::make_UnitQuad()},
    mProgram{
        makeLinkedProgram({
            {GL_VERTEX_SHADER, gPassthroughVertexShader},
            {GL_FRAGMENT_SHADER, fade::gFragmentShader}
    })}
{
    setUniform(mProgram, "image", gTextureUnit); 
}


void Fade::apply(math::sdr::Rgba aFadeToColor, 
                 const Texture & aSource,
                 FrameBuffer & aTarget)
{
    setUniform(mProgram, "fadeToColor", to_hdr(aFadeToColor)); 

    // Bind the VAO and activate the program.
    activate(mUnitQuad, mProgram);
    Guard activationGuard{ [&](){deactivate(mUnitQuad, mProgram);} };

    // Active the texture unit that is mapped to the programs sampler,
    // and bind the source texture.
    auto activeTexUnit = activateTextureUnitGuard(gTextureUnit);
    ScopedBind boundTexture{aSource};

    // Bind the target.
    ScopedBind boundFrameBuffer{aTarget};

    glDrawArrays(GL_TRIANGLE_STRIP, 0, detail::gQuadVerticeCount);
}


} // namespace graphics
} // namespace ad
