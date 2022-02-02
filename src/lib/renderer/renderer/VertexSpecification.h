#pragma once

#include "gl_helpers.h"
#include "GL_Loader.h"
#include "MappedGL.h"
#include "Range.h"

#include <handy/Guard.h>

#include <gsl/span>

#include <type_traits>
#include <vector>


namespace ad {
namespace graphics {


struct [[nodiscard]] VertexArrayObject : public ResourceGuard<GLuint>
{
    /// \note Deleting a bound VAO reverts the binding to zero
    VertexArrayObject() :
        ResourceGuard<GLuint>{reserve(glGenVertexArrays),
                              [](GLuint aIndex){glDeleteVertexArrays(1, &aIndex);}}
    {}
};


inline void bind(const VertexArrayObject & aVertexArray)
{
    glBindVertexArray(aVertexArray);
}

// TODO Ad 2022/02/02: Is it a good idea to "expect" the object to unbind
// when the underlying unbinding mechanism does not use it (just reset a default)? 
inline void unbind(const VertexArrayObject & aVertexArray)
{
    glBindVertexArray(0);
}


/// \TODO understand when glDisableVertexAttribArray should actually be called
///       (likely not specially before destruction, but more when rendering other objects
///        since it is a current(i.e. global) VAO state)
/// \Note Well note even that: Activated vertex attribute array are per VAO, so changing VAO
///       Already correctly handles that.
struct [[nodiscard]] VertexBufferObject : public ResourceGuard<GLuint>
{
    VertexBufferObject() :
        ResourceGuard<GLuint>{reserve(glGenBuffers),
                              [](GLuint aIndex){glDeleteBuffers(1, &aIndex);}}
    {}
};


struct [[nodiscard]] IndexBufferObject : public ResourceGuard<GLuint>
{
    IndexBufferObject() :
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


inline void bind(const VertexSpecification & aVertexSpecification)
{
    bind(aVertexSpecification.mVertexArray);
}


inline void unbind(const VertexSpecification & aVertexSpecification)
{
    unbind(aVertexSpecification.mVertexArray);
}



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


/***
 *
 * Vertex Buffer
 *
 ***/

/// \brief Attach an existing VertexBuffer to an exisiting VertexArray,
/// without providing initial data.
void attachVertexBuffer(const VertexBufferObject & aVertexBuffer,
                        const VertexArrayObject & aVertexArray,
                        AttributeDescriptionList aAttributes,
                        GLsizei aStride,
                        GLuint aAttributeDivisor = 0);

/// \brief This overload deduces the stride from T_vertex.
template <class T_vertex>
void attachVertexBuffer(const VertexBufferObject & aVertexBuffer,
                        const VertexArrayObject & aVertexArray,
                        AttributeDescriptionList aAttributes,
                        GLuint aAttributeDivisor = 0)
{
    return attachVertexBuffer(aVertexBuffer, aVertexArray, aAttributes, sizeof(T_vertex), aAttributeDivisor);
}

/// \brief Intialize a VertexBufferObject, without providing initial data.
///
/// This is an extension of `attachVertexBuffer()`, which constructs the vertex buffer it attaches,
/// instead of expecting it as argument.
VertexBufferObject initVertexBuffer(const VertexArrayObject & aVertexArray,
                                    AttributeDescriptionList aAttributes,
                                    GLsizei aStride,
                                    GLuint aAttributeDivisor = 0);


/// \brief This overload deduces the stride from T_vertex.
template <class T_vertex>
VertexBufferObject initVertexBuffer(const VertexArrayObject & aVertexArray,
                                    AttributeDescriptionList aAttributes,
                                    GLuint aAttributeDivisor = 0)
{
    return initVertexBuffer(aVertexArray, aAttributes, sizeof(T_vertex), aAttributeDivisor);
}

/// \brief Create a VertexBufferObject with provided attributes, load it with data,
/// and associate the data to attributes of `aVertexArray`.
///
/// This is an extension of `initVertexBuffer()`, which loads data into the initialized vertex buffer.
///
/// \param aAttribute describes the associaiton.
///
/// \note This is the lowest level overload, with explicit attribute description and raw data pointer.
/// Other overloads end-up calling it.
VertexBufferObject loadVertexBuffer(const VertexArrayObject & aVertexArray,
                                    AttributeDescriptionList aAttributes,
                                    GLsizei aStride,
                                    size_t aSize,
                                    const GLvoid * aData,
                                    GLuint aAttributeDivisor = 0);


/// \brief This overload deduces the stride and size from T_vertex,
/// which could itself be deduced from the provided span.
template <class T_vertex>
VertexBufferObject loadVertexBuffer(const VertexArrayObject & aVertexArray,
                                    const AttributeDescriptionList & aAttributes,
                                    const gsl::span<T_vertex> aVertices,
                                    GLuint aAttributeDivisor = 0)
{
    return loadVertexBuffer(aVertexArray,
                            aAttributes,
                            sizeof(T_vertex),
                            aVertices.size_bytes(),
                            aVertices.data(),
                            aAttributeDivisor);
}


/// \brief Create a VertexBufferObject and load it with provided data.
/// But do **not** attach it to a VertexArrayObject / do **not** associate the vertex data to attributes.
///
/// \note Attachment to a VertexArrayObject as well as attributes association might be done later with
/// `attachVertexBuffer()`.
template <class T_vertex>
VertexBufferObject loadUnattachedVertexBuffer(gsl::span<const T_vertex> aVertices,
                                              GLenum aHint = GL_STATIC_DRAW)
{
    VertexBufferObject vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, aVertices.size_bytes(), aVertices.data(), aHint);
    return vbo;
}


/// \brief High-level function directly appending a loaded VertexBuffer to a VertexSpecification.
///
/// This is an extension to `loadVertexBuffer()`, which appends the loaded vertex buffer to `aSpecification`.
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


/***
 *
 * Index Buffer
 *
 ***/

/// \brief Attach an existing IndexBuffer to an exisiting VertexArray,
/// without providing initial data.
inline void attachIndexBuffer(const IndexBufferObject & aIndexBuffer,
                              const VertexArrayObject & aVertexArray)
{
    glBindVertexArray(aVertexArray);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, aIndexBuffer);
}


/// \brief Intialize and attach an IndexBufferObject, without providing initial data.
///
/// This is an extension of `attachIndexBuffer()`, which constructs the index buffer it attaches,
/// instead of expecting it as argument.
inline IndexBufferObject initIndexBuffer(const VertexArrayObject & aVertexArray)
{
    IndexBufferObject ibo;
    attachIndexBuffer(ibo, aVertexArray);
    return ibo;
}


/// \brief Initialize, attach and load data into an IndexBufferObject.
///
/// This is an extension of `initIndexBuffer()`, which loads data into the initialized vertex buffer.
template <class T_index>
IndexBufferObject loadIndexBuffer(const VertexArrayObject & aVertexArray,
                                  const gsl::span<T_index> aIndices,
                                  const BufferHint aHint)
{
    IndexBufferObject ibo = initIndexBuffer(aVertexArray);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, aIndices.size_bytes(), aIndices.data(), getGLBufferHint(aHint));
    return ibo;
}


/***
 * Buffer re-specification
 *
 * see: https://www.khronos.org/opengl/wiki/Buffer_Object_Streaming#Buffer_re-specification
 ***/

/// \brief Respecify content of a vertex buffer.
inline void respecifyBuffer(const VertexBufferObject & aVBO, const GLvoid * aData, GLsizei aSize)
{
    glBindBuffer(GL_ARRAY_BUFFER, aVBO);

    // Orphan the previous buffer
    glBufferData(GL_ARRAY_BUFFER, aSize, NULL, GL_STATIC_DRAW);

    // Copy value to new buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, aSize, aData);
}


/// \brief Respecify content of an index buffer.
inline void respecifyBuffer(const IndexBufferObject & aIBO, const GLvoid * aData, GLsizei aSize)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, aIBO);

    // Orphan the previous buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, aSize, NULL, GL_STATIC_DRAW);

    // Copy value to new buffer
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, aSize, aData);
}


/// \brief Overload accepting a span of generic values, instead of low-level void pointer.
/// It works with both vertex and index buffers.
template <class T_values, class T_buffer>
void respecifyBuffer(const T_buffer & aBufferObject, const gsl::span<T_values> aValues)
{
    respecifyBuffer(aBufferObject,
                    aValues.data(),
                    static_cast<GLsizei>(aValues.size_bytes()));
}


/// \brief Respecify a vertex buffer with the exactly same size (allowing potential optimizations).
///
/// \attention This is undefined behaviour is aData does not point to at least the same amount
/// of data that was present before in the re-specified vertex buffer.
inline void respecifyBufferSameSize(const VertexBufferObject & aVBO, const GLvoid * aData)
{
    GLint size;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

    respecifyBuffer(aVBO, aData, size);
}


} // namespace graphics
} // namespace ad
