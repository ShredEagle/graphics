#pragma once

#include <math/Vector.h>

#include <glad/glad.h>

namespace ad {

using Color = math::Vec<3, GLubyte>;

// There is no literal suffix for unsigned char, making it a pain to instantiate a Color
// from literal values. Provide a helper function for this situation.
inline constexpr Color rgb(GLubyte r, GLubyte g, GLubyte b)
{
    return Color(r, g, b);
}

} // namespace ad
