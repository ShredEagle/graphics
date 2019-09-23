#pragma once

#include <math/Vector.h>

#include <glad/glad.h>

namespace ad {

template <class T>
using Vec2=math::Vec2<T>;
/// \todo Have a dedicated position type
using Position=math::Vec2<GLint>;

template <class T>
using Size2 = math::Dimension2<T>;

using Color = math::Vec3<GLubyte>;

} // namespace ad
