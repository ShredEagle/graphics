#pragma once


#include "GL_Loader.h"

#include <math/Color.h>


namespace ad {
namespace graphics {

#define MAP(trait, type, enumval)       \
    template <> struct trait<type>      \
    { static constexpr GLenum enumerator = enumval; };

//
// Built-in types
//
template <class T_scalar>
struct MappedGL;

MAP(MappedGL, GLfloat, GL_FLOAT);
MAP(MappedGL, GLdouble, GL_DOUBLE);
MAP(MappedGL, GLbyte, GL_BYTE);
MAP(MappedGL, GLubyte, GL_UNSIGNED_BYTE);
MAP(MappedGL, GLshort, GL_SHORT);
MAP(MappedGL, GLushort, GL_UNSIGNED_SHORT);
MAP(MappedGL, GLint, GL_INT);
MAP(MappedGL, GLuint, GL_UNSIGNED_INT);


//
// Pixel formats
//
template <class T_pixel>
struct MappedPixel;

MAP(MappedPixel, math::sdr::Rgb, GL_RGB);
MAP(MappedPixel, math::sdr::Rgba, GL_RGBA);
MAP(MappedPixel, math::hdr::Rgb_f, GL_RGB);

template <class T_pixel>
constexpr GLenum MappedPixel_v = MappedPixel<T_pixel>::enumerator;


#undef MAP


//
// glGet parameter names for texture bindings
//
template <GLenum N_textureTarget>
struct MappedTextureBindingGL;

#define TEXTURECASE(textureTarget, paramName)   \
    case textureTarget:                         \
        return paramName;


constexpr GLenum getGLMappedTextureBinding(GLenum aTextureTarget) 
{
    switch(aTextureTarget)
    {
        TEXTURECASE(GL_TEXTURE_1D, GL_TEXTURE_BINDING_1D);
        TEXTURECASE(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_BINDING_1D_ARRAY);
        TEXTURECASE(GL_TEXTURE_2D, GL_TEXTURE_BINDING_2D);
        TEXTURECASE(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BINDING_2D_ARRAY);
        TEXTURECASE(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_BINDING_2D_MULTISAMPLE);
        TEXTURECASE(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY);
        TEXTURECASE(GL_TEXTURE_3D, GL_TEXTURE_BINDING_3D);
        TEXTURECASE(GL_TEXTURE_BUFFER, GL_TEXTURE_BINDING_BUFFER);
        TEXTURECASE(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BINDING_CUBE_MAP);
        TEXTURECASE(GL_TEXTURE_RECTANGLE, GL_TEXTURE_BINDING_RECTANGLE);
    default:
        throw std::domain_error{"Invalid texture target."};
    }
}


#undef TEXTURECASE


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

} // namespace graphics
} // namespace ad
