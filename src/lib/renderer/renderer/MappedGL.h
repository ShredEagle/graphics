#pragma once

#include <glad/glad.h>

namespace ad {

template <class T_scalar>
struct MappedGL;

#define MAP(type, enumval)              \
    template <> struct MappedGL<type>   \
    { static const GLenum enumerator = enumval; };

MAP(GLfloat, GL_FLOAT);
MAP(GLubyte, GL_UNSIGNED_BYTE);
MAP(GLbyte, GL_BYTE);
MAP(GLint, GL_INT);

#undef MAP

} // namespace ad
