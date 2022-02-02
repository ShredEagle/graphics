#pragma once


#include "../commons.h"

#include <renderer/Shading.h>
#include <renderer/VertexSpecification.h>

namespace ad {
namespace graphics {


class Fade
{
public:
    Fade();

    void apply(math::sdr::Rgba aFadeToColor,
               const Texture & aSource,
               FrameBuffer & aTarget);

private:
    static constexpr GLsizei gTextureUnit = 1;

    VertexSpecification mUnitQuad;
    Program mProgram;
};


} // namespace graphics
} // namespace ad
