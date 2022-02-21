#include "Texture.h"

#include "FrameBuffer.h"

#include "GL_Loader.h"


namespace ad {
namespace graphics {

void clear(const Texture & aTexture, math::hdr::Rgba aClearValue)
{
    std::array<GLfloat, 4> clear{
        aClearValue.r(),
        aClearValue.g(),
        aClearValue.b(),
        aClearValue.a(),
    };

    if (!GL_ARB_clear_texture)
    {
        FrameBuffer fb;
        attachImage(fb, aTexture);
        glClearBufferfv(GL_COLOR, 0, clear.data());
    }
    else
    {
        bind_guard bound(aTexture);
        glClearTexImage(aTexture, 0, GL_RGBA, GL_FLOAT, clear.data());
    }
}

} // namespace graphics
} // namespace ad
