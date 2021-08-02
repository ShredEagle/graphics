#pragma once

#include "gl_helpers.h"
#include "GL_Loader.h"
#include "MappedGL.h"
#include "Range.h"

#include <handy/Guard.h>

#include <gsl/span>

#include <type_traits>
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


/// \brief A VertexArray with a vector of VertexBuffers
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


/// \brief Describes the attribute access from the shader (layout id, value type, normalization)
struct Attribute
{
    enum class Access
    {
        Float,
        Integer,
    };

    constexpr Attribute(GLuint aValue) :
        mIndex(aValue)
    {}

    constexpr Attribute(GLuint aValue, Access aAccess, bool aNormalize=false) :
        mIndex(aValue),
        mTypeInShader(aAccess),
        mNormalize(aNormalize)
    {}

    GLuint mIndex; // index to match in vertex shader.
    Access mTypeInShader{Access::Float}; // destination data type
    bool mNormalize{false}; // if destination is float and source is integral, should it be normalized (value/max_value)
};

/// \brief The complete description of an attribute expected by OpenGL
struct AttributeDescription : public Attribute
{
    GLuint mDimension;  // from 1 to 4 (explicit distinct attributes must be used for matrix data)
    size_t mOffset;     // offset for the attribute within the vertex data structure (interleaved)
    GLenum mDataType;   // attribute source data type
};

std::ostream & operator<<(std::ostream &aOut, const AttributeDescription & aDescription);

typedef std::initializer_list<AttributeDescription> AttributeDescriptionList;


/// \brief Load vertex data from `aVertices` into the returned `VertexBufferObject`,
/// and associate the data to attributes of `aVertexArray`.
///
/// The association is described by `aAttributes`.
template <class T_vertex>
VertexBufferObject loadVertexBuffer(const VertexArrayObject & aVertexArray,
                                    const AttributeDescriptionList & aAttributes,
                                    const gsl::span<T_vertex> aVertices,
                                    GLuint aAttributeDivisor = 0)
{
    glBindVertexArray(aVertexArray);

    if (aAttributeDivisor)
    {
        for(const auto & attribute : aAttributes)
        {
            glVertexAttribDivisor(attribute.mIndex, aAttributeDivisor);
        }
    }

    return makeLoadedVertexBuffer(aAttributes, aVertices);
}


template <class T_vertex>
void appendToVertexSpecification(VertexSpecification & aSpecification,
                                 const AttributeDescriptionList & aAttributes,
                                 const gsl::span<T_vertex> aVertices,
                                 GLuint aAttributeDivisor = 0)
{
    aSpecification.mVertexBuffers.push_back(
            loadVertexBuffer(aSpecification.mVertexArray,
                             aAttributes,
                             std::move(aVertices),
                             aAttributeDivisor));
}

/// \brief Create a VertexBufferObject with provided attributes, load it with data.
///
/// This is the lowest level overload, with explicit attribute description and raw data pointer.
/// Other overloads end-up calling it.
VertexBufferObject makeLoadedVertexBuffer(AttributeDescriptionList aAttributes,
                                          GLsizei aStride,
                                          size_t aSize,
                                          const GLvoid * aData);

template <class T_vertex>
VertexBufferObject makeLoadedVertexBuffer(AttributeDescriptionList aAttributes,
                                          const gsl::span<T_vertex> aVertices)
{
    return makeLoadedVertexBuffer(aAttributes,
                                  sizeof(T_vertex),
                                  aVertices.size_bytes(),
                                  aVertices.data());
}


/***
 * Buffer re-specification
 *
 * see: https://www.khronos.org/opengl/wiki/Buffer_Object_Streaming#Buffer_re-specification
 ***/

inline void respecifyBuffer(const VertexBufferObject & aVBO, const GLvoid * aData, GLsizei aSize)
{
    glBindBuffer(GL_ARRAY_BUFFER, aVBO);

    // Orphan the previous buffer
    glBufferData(GL_ARRAY_BUFFER, aSize, NULL, GL_STATIC_DRAW);

    // Copy value to new buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, aSize, aData);
}


/// \brief Respecify the buffer with the same size (allowing potential optimizations)
inline void respecifyBufferSameSize(const VertexBufferObject & aVBO, const GLvoid * aData)
{
    GLint size;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

    respecifyBuffer(aVBO, aData, size);
}

template <class T_vertex>
void respecifyBuffer(const VertexBufferObject & aVBO, const gsl::span<T_vertex> aVertices)
{
    respecifyBuffer(aVBO,
                    aVertices.data(),
                    static_cast<GLsizei>(aVertices.size_bytes()));
}

} // namespace ad
