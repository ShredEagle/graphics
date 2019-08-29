#pragma once

#include "gl_helpers.h"

#include <handy/Guard.h>

#include <glad/glad.h>

#include <vector>

namespace ad
{

struct [[nodiscard]] VertexArrayObject : public ResourceGuard<GLuint>
{
    /// \note Deleting a bound VAO reverts the binding to zero
    VertexArrayObject() :
        ResourceGuard<GLuint>{reserve(glGenVertexArrays),
                              [](GLuint aIndex){glDeleteVertexArrays(1, &aIndex);}}
    {}
};

/// \TODO understand when glDisableVertexAttribArray should actually be called
///       (likely not specially before destruction, but more when rendering other objects
///        since it is a current(i.e. global) VAO state)
/// \Note Well note even that: Activated vertex attribute array are per VAO, so changing VAO
//        Already correctly handles that.
struct [[nodiscard]] VertexBufferObject : public ResourceGuard<GLuint>
{
    VertexBufferObject() :
        ResourceGuard<GLuint>{reserve(glGenBuffers),
                              [](GLuint aIndex){glDeleteBuffers(1, &aIndex);}}
    {}
};


struct [[nodiscard]] VertexSpecification
{
    VertexSpecification(VertexArrayObject aVertexArray={},
                        std::vector<VertexBufferObject> aVertexBuffers={}) :
        mVertexArray{std::move(aVertexArray)},
        mVertexBuffers{std::move(aVertexBuffers)}
    {}

    VertexArrayObject mVertexArray;
    std::vector<VertexBufferObject> mVertexBuffers;
};


template <class T_scalar>
struct MappedGL;

template <> struct MappedGL<GLfloat>
{ static const GLenum enumerator = GL_FLOAT; };

template <> struct MappedGL<GLubyte>
{ static const GLenum enumerator = GL_UNSIGNED_BYTE; };


template <class T_element, int N_vertices, int N_attributeDimension>
VertexBufferObject makeLoadedVertexBuffer(
        GLuint aAttributeId,
        T_element (& data)[N_vertices][N_attributeDimension])
{
    VertexBufferObject vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

    glVertexAttribPointer(aAttributeId,
                          N_attributeDimension,
                          MappedGL<T_element>::enumerator,
                          GL_FALSE,
                          0,
                          0);
    glEnableVertexAttribArray(aAttributeId);

    return vbo;
}

struct AttributeDescription
{
    GLuint mIndex;
    GLuint mDimension;
    size_t mOffset;
    /// \TODO clever template tricks should allow to rework this function and deduce that
    GLenum mDataType;
    bool mNormalize{false};
};

template <class T_data>
VertexBufferObject makeLoadedVertexBuffer(std::vector<AttributeDescription> aAttributes,
                                          GLsizei aStride,
                                          size_t aSize,
                                          const T_data & aData)
{
    VertexBufferObject vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, aSize, aData, GL_STATIC_DRAW);

    for (const auto & attribute : aAttributes)
    {
        glVertexAttribPointer(attribute.mIndex,
                              attribute.mDimension,
                              attribute.mDataType,
                              attribute.mNormalize,
                              aStride,
                              reinterpret_cast<const void*>(attribute.mOffset));
        glEnableVertexAttribArray(attribute.mIndex);
    }

    return vbo;
}

// Sadly C++ 20, but it would handle nicely arrays and vectors
//template <class T_contiguousIterator>
//std::enable_if<std::iterator_traits<T_contiguousIterator>::iterator_category == ...>
template <class T_element>
VertexBufferObject makeLoadedVertexBuffer(std::vector<AttributeDescription> aAttributes,
                                          typename std::vector<T_element>::const_iterator aFirst,
                                          typename std::vector<T_element>::const_iterator aLast)
{
    return makeLoadedVertexBuffer(std::move(aAttributes),
                                  sizeof(T_element),
                                  sizeof(T_element) * std::distance(aFirst, aLast),
                                  &(*aFirst));
}

/// \see: https://www.khronos.org/opengl/wiki/Buffer_Object_Streaming#Buffer_re-specification
template <class T_data>
void respecifyBuffer(VertexBufferObject & aVBO, const T_data & aData)
{
    glBindBuffer(GL_ARRAY_BUFFER, aVBO);

    // Orphan the previous buffer
    GLint size;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);

    // Copy value to new buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, aData);
}


} // namespace ad
