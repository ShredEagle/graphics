#include "Texture.h"

#include "FrameBuffer.h"

#include "GL_Loader.h"


namespace ad {
namespace graphics {

void clear(const Texture & aTexture, math::hdr::Rgba_f aClearValue)
{
    if (!GL_ARB_clear_texture)
    {
        FrameBuffer fb;
        attachImage(fb, aTexture);
        glClearBufferfv(GL_COLOR, 0, aClearValue.data());
    }
    else
    {
        ScopedBind bound(aTexture);
        glClearTexImage(aTexture, 0, GL_RGBA, GL_FLOAT, aClearValue.data());
    }
}

} // namespace graphics
} // namespace ad
