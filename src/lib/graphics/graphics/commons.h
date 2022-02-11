#pragma once

#include <handy/Bitmask.h>

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


// <renderer> forward declarations
struct FrameBuffer;
struct Program;
struct Texture;
struct VertexSpecification;


enum class Mirroring
{
    None = 0,
    FlipHorizontal = (1 << 1),
    FlipVertical = (1 << 2),
};


} // namespace graphics


template <>
struct is_bitmask<graphics::Mirroring> : public std::true_type
{};


} // namespace ad
