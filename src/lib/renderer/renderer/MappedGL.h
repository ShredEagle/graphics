#pragma once

#include "GL_Loader.h"

namespace ad {

template <class T_scalar>
struct MappedGL;

#define MAP(type, enumval)              \
    template <> struct MappedGL<type>   \
    { static const GLenum enumerator = enumval; };

MAP(GLfloat, GL_FLOAT);
MAP(GLdouble, GL_DOUBLE);
MAP(GLbyte, GL_BYTE);
MAP(GLubyte, GL_UNSIGNED_BYTE);
MAP(GLshort, GL_SHORT);
MAP(GLushort, GL_UNSIGNED_SHORT);
MAP(GLint, GL_INT);
MAP(GLuint, GL_UNSIGNED_INT);

#undef MAP


enum class BufferHint
{
    StreamDraw,
    StreamRead,
    StreamCopy,
    StaticDraw,
    StaticRead,
    StaticCopy,
    DynamicDraw,
    DynamicRead,
    DynamicCopy,
};

template <BufferHint>
struct BufferHintGL;


#define HINT(customenum, glenum)                \
    template <> struct BufferHintGL<customenum> \
    { static constexpr GLenum enumerator = glenum; };

HINT(BufferHint::StreamDraw,  GL_STREAM_DRAW);
HINT(BufferHint::StreamRead,  GL_STREAM_READ);
HINT(BufferHint::StreamCopy,  GL_STREAM_COPY);
HINT(BufferHint::StaticDraw,  GL_STATIC_DRAW);
HINT(BufferHint::StaticRead,  GL_STATIC_READ);
HINT(BufferHint::StaticCopy,  GL_STATIC_COPY);
HINT(BufferHint::DynamicDraw, GL_DYNAMIC_DRAW);
HINT(BufferHint::DynamicRead, GL_DYNAMIC_READ);
HINT(BufferHint::DynamicCopy, GL_DYNAMIC_COPY);

#undef HINT

#define HINTCASE(customenum, glenum)    \
    case customenum:        \
        return glenum;

constexpr GLenum getGLBufferHint(const BufferHint aBufferHint) 
{
    switch(aBufferHint)
    {
        HINTCASE(BufferHint::StreamDraw,  GL_STREAM_DRAW);
        HINTCASE(BufferHint::StreamRead,  GL_STREAM_READ);
        HINTCASE(BufferHint::StreamCopy,  GL_STREAM_COPY);
        HINTCASE(BufferHint::StaticDraw,  GL_STATIC_DRAW);
        HINTCASE(BufferHint::StaticRead,  GL_STATIC_READ);
        HINTCASE(BufferHint::StaticCopy,  GL_STATIC_COPY);
        HINTCASE(BufferHint::DynamicDraw, GL_DYNAMIC_DRAW);
        HINTCASE(BufferHint::DynamicRead, GL_DYNAMIC_READ);
        HINTCASE(BufferHint::DynamicCopy, GL_DYNAMIC_COPY);
    default:
        throw std::domain_error{"Invalid BufferHint enumerator."};
    }
}

#undef HINTCASE

template <BufferHint N_customEnum>
constexpr GLenum BufferHint_v = BufferHintGL<N_customEnum>::enumerator;

} // namespace ad
