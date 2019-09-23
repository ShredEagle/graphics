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

template <> struct MappedGL<GLint>
{ static const GLenum enumerator = GL_INT; };


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

enum class ShaderAccess
{
    Float,
    Integer,
};

struct AttributeDescription
{
    GLuint mIndex;
    GLuint mDimension;
    size_t mOffset;
    /// \TODO clever template tricks should allow to rework this function and deduce that
    GLenum mDataType;
    ShaderAccess typeInShader{ShaderAccess::Float};
    bool mNormalize{false};
};

/// \todo Enhance those signatures
///       Better automatic deduction
///       More permissive ranges for attributes and data (data could benefit from "ContiguousStorage" in C++20)
/// \todo What is the point of the template param here?
template <class T_data>
VertexBufferObject makeLoadedVertexBuffer(std::vector<const AttributeDescription> aAttributes,
                                          GLsizei aStride,
                                          size_t aSize,
                                          const T_data & aData)
{
    VertexBufferObject vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, aSize, aData, GL_STATIC_DRAW);

    for (const auto & attribute : aAttributes)
    {
        switch(attribute.typeInShader)
        {
            case ShaderAccess::Float :
            {
                glVertexAttribPointer(attribute.mIndex,
                                      attribute.mDimension,
                                      attribute.mDataType,
                                      attribute.mNormalize,
                                      aStride,
                                      reinterpret_cast<const void*>(attribute.mOffset));
                break;
            }
            case ShaderAccess::Integer :
            {
                glVertexAttribIPointer(attribute.mIndex,
                                       attribute.mDimension,
                                       attribute.mDataType,
                                       aStride,
                                       reinterpret_cast<const void*>(attribute.mOffset));
                break;
            }
        }

        glEnableVertexAttribArray(attribute.mIndex);
    }

    return vbo;
}

// Sadly C++ 20, but it would handle nicely arrays and vectors
//template <class T_contiguousIterator>
//std::enable_if<std::iterator_traits<T_contiguousIterator>::iterator_category == ...>
template <class T_element>
VertexBufferObject makeLoadedVertexBuffer(std::vector<const AttributeDescription> aAttributes,
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
void respecifyBuffer(const VertexBufferObject & aVBO, const T_data & aData, GLsizei aSize)
{
    glBindBuffer(GL_ARRAY_BUFFER, aVBO);

    // Orphan the previous buffer
    glBufferData(GL_ARRAY_BUFFER, aSize, NULL, GL_STATIC_DRAW);

    // Copy value to new buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, aSize, aData);
}

/// \brief Respecify the buffer with the same size (allowing potential optimizations)
template <class T_data>
void respecifyBuffer(const VertexBufferObject & aVBO, const T_data & aData)
{
    GLint size;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

    respecifyBuffer(aVBO, aData, size);
}

} // namespace ad
