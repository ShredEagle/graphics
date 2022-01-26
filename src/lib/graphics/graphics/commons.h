#pragma once

#include <math/Color.h>

#include <renderer/GL_Loader.h>

namespace ad {
namespace graphics {


namespace sprite {
    struct LoadedAtlas;
} // namespace sprite

// TODO It is unclear to me if it would be incorrect to use sdr::Rgb (i.e. Rgb_base<std::uint8_t>) 
// instead of GLubyte (which matches exactly to GL_UNSIGNED_BYTE).
using Color = math::Rgb_base<GLubyte>;


} // namespace graphics
} // namespace ad
