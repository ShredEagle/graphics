#pragma once


#include "GL_Loader.h"

#include <math/Color.h>


namespace ad {
namespace graphics {

#define MAP(trait, gltype, enumval)                     \
    template <> struct trait<gltype>                    \
    { static constexpr GLenum enumerator = enumval; };  

#define REVERSE_MAP(trait, gltype, enumval)             \
    template <> struct trait##_r<enumval>               \
    { using type = gltype; };

#define MAP_AND_REVERSE(trait, gltype, enumval)         \
    MAP(trait, gltype, enumval)                         \
    REVERSE_MAP(trait, gltype, enumval)

//
// Built-in types
//
template <class T_scalar>
struct MappedGL;

template <class T_scalar>
constexpr GLenum MappedGL_v = MappedGL<T_scalar>::enumerator;

template <GLuint N_typeEnum>
struct MappedGL_r;

template <GLuint N_typeEnum>
using GLFromTypeEnum_t = typename MappedGL_r<N_typeEnum>::type;

MAP_AND_REVERSE(MappedGL, GLfloat, GL_FLOAT);
MAP_AND_REVERSE(MappedGL, GLdouble, GL_DOUBLE);
MAP_AND_REVERSE(MappedGL, GLbyte, GL_BYTE);
MAP_AND_REVERSE(MappedGL, GLubyte, GL_UNSIGNED_BYTE);
MAP_AND_REVERSE(MappedGL, GLshort, GL_SHORT);
MAP_AND_REVERSE(MappedGL, GLushort, GL_UNSIGNED_SHORT);
MAP_AND_REVERSE(MappedGL, GLint, GL_INT);
MAP_AND_REVERSE(MappedGL, GLuint, GL_UNSIGNED_INT);
// Only reverse is possible, because GLboolean is the same type as GLubyte
REVERSE_MAP(MappedGL, GLboolean, GL_BOOL);


#define TYPEENUMCASE(enumval)           \
    case enumval:                       \
        return sizeof(GLFromTypeEnum_t<enumval>);


constexpr GLenum getByteSize(GLenum aTypeEnum) 
{
    switch(aTypeEnum)
    {
        TYPEENUMCASE(GL_FLOAT);
        TYPEENUMCASE(GL_DOUBLE);
        TYPEENUMCASE(GL_BYTE);
        TYPEENUMCASE(GL_UNSIGNED_BYTE);
        TYPEENUMCASE(GL_SHORT);
        TYPEENUMCASE(GL_UNSIGNED_SHORT);
        TYPEENUMCASE(GL_INT);
        TYPEENUMCASE(GL_UNSIGNED_INT);
        TYPEENUMCASE(GL_BOOL);
    default:
        throw std::domain_error{"Invalid type enumerator."};
    }
}

#undef TYPEENUMCASE


#define GLENUMCASE(enumval)           \
    case enumval:                       \
        return #enumval;


inline std::string to_string(GLenum aGLEnumerator) 
{
    switch(aGLEnumerator)
    {
        //
        // Types
        //
        GLENUMCASE(GL_FLOAT);
        GLENUMCASE(GL_DOUBLE);
        GLENUMCASE(GL_BYTE);
        GLENUMCASE(GL_UNSIGNED_BYTE);
        GLENUMCASE(GL_SHORT);
        GLENUMCASE(GL_UNSIGNED_SHORT);
        GLENUMCASE(GL_INT);
        GLENUMCASE(GL_UNSIGNED_INT);
        GLENUMCASE(GL_BOOL);

        //
        // Targets
        //
        GLENUMCASE(GL_ARRAY_BUFFER);
        GLENUMCASE(GL_ATOMIC_COUNTER_BUFFER);
        GLENUMCASE(GL_COPY_READ_BUFFER);
        GLENUMCASE(GL_COPY_WRITE_BUFFER);
        //TYPEENUMCASE(GL_DISPATCH_INDIRECT_BUFFER);
        GLENUMCASE(GL_DRAW_INDIRECT_BUFFER);
        GLENUMCASE(GL_ELEMENT_ARRAY_BUFFER);
        GLENUMCASE(GL_PIXEL_PACK_BUFFER);
        GLENUMCASE(GL_PIXEL_UNPACK_BUFFER);
        //TYPEENUMCASE(GL_QUERY_BUFFER);
        GLENUMCASE(GL_SHADER_STORAGE_BUFFER);
        GLENUMCASE(GL_TEXTURE_BUFFER);
        GLENUMCASE(GL_TRANSFORM_FEEDBACK_BUFFER);
        GLENUMCASE(GL_UNIFORM_BUFFER);

    default:
        throw std::domain_error{"Invalid GL enumerator."};
    }
}

#undef GLENUMCASE

//
// Pixel formats
//
template <class T_pixel>
struct MappedPixel;

template <class T_pixel>
constexpr GLenum MappedPixel_v = MappedPixel<T_pixel>::enumerator;

MAP(MappedPixel, math::sdr::Rgb, GL_RGB);
MAP(MappedPixel, math::sdr::Rgba, GL_RGBA);
MAP(MappedPixel, math::hdr::Rgb_f, GL_RGB);

template <class T_pixel>
struct MappedSizedPixel;

template <class T_pixel>
constexpr GLenum MappedSizedPixel_v = MappedSizedPixel<T_pixel>::enumerator;

MAP(MappedSizedPixel, math::sdr::Rgb, GL_RGB8);
MAP(MappedSizedPixel, math::sdr::Rgba, GL_RGBA8);

#undef MAP_AND_REVERSE
#undef MAP


//
// glGet parameter names for texture bindings
//
template <GLenum N_textureTarget>
struct MappedTextureBindingGL;

#define BINDINGCASE(target, paramName)   \
    case target:                         \
        return paramName;


constexpr GLenum getGLMappedTextureBinding(GLenum aTextureTarget) 
{
    switch(aTextureTarget)
    {
        BINDINGCASE(GL_TEXTURE_1D, GL_TEXTURE_BINDING_1D);
        BINDINGCASE(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_BINDING_1D_ARRAY);
        BINDINGCASE(GL_TEXTURE_2D, GL_TEXTURE_BINDING_2D);
        BINDINGCASE(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BINDING_2D_ARRAY);
        BINDINGCASE(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_BINDING_2D_MULTISAMPLE);
        BINDINGCASE(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY);
        BINDINGCASE(GL_TEXTURE_3D, GL_TEXTURE_BINDING_3D);
        BINDINGCASE(GL_TEXTURE_BUFFER, GL_TEXTURE_BINDING_BUFFER);
        BINDINGCASE(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BINDING_CUBE_MAP);
        BINDINGCASE(GL_TEXTURE_RECTANGLE, GL_TEXTURE_BINDING_RECTANGLE);
    default:
        throw std::domain_error{"Invalid texture target."};
    }
}

constexpr GLenum getGLMappedBufferBinding(GLenum aBufferTarget) 
{
    switch(aBufferTarget)
    {
        BINDINGCASE(GL_ARRAY_BUFFER, GL_ARRAY_BUFFER_BINDING);
        BINDINGCASE(GL_DRAW_INDIRECT_BUFFER, GL_DRAW_INDIRECT_BUFFER_BINDING);
        BINDINGCASE(GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER_BINDING);
        BINDINGCASE(GL_PIXEL_PACK_BUFFER, GL_PIXEL_PACK_BUFFER_BINDING);
        BINDINGCASE(GL_PIXEL_UNPACK_BUFFER, GL_PIXEL_UNPACK_BUFFER_BINDING);
        BINDINGCASE(GL_SHADER_STORAGE_BUFFER, GL_SHADER_STORAGE_BUFFER_BINDING);
        BINDINGCASE(GL_TRANSFORM_FEEDBACK_BUFFER, GL_TRANSFORM_FEEDBACK_BUFFER_BINDING);
        BINDINGCASE(GL_UNIFORM_BUFFER, GL_UNIFORM_BUFFER_BINDING);

        BINDINGCASE(GL_DRAW_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER_BINDING);
        BINDINGCASE(GL_READ_FRAMEBUFFER, GL_READ_FRAMEBUFFER_BINDING);

    default:
        throw std::domain_error{"Invalid buffer target."};
    }
}

#undef BINDINGCASE


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
