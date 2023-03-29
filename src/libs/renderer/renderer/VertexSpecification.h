#pragma once

#include "AttributeDimension.h"
#include "BufferBase.h"
#include "GL_Loader.h"
#include "MappedGL.h"


#include <handy/Guard.h>

#include <span>
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


inline void bind(Name<VertexArrayObject> aVertexArray)
{
    glBindVertexArray(aVertexArray);
}


inline void unbind(const VertexArrayObject & /*aVertexArray*/)
{
    glBindVertexArray(0);
}


inline Name<VertexArrayObject> getBound(const VertexArrayObject &)
{
    GLint current;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current);
    return Name<VertexArrayObject>{(GLuint)current, Name<VertexArrayObject>::UnsafeTag{}};
}


using VertexBufferObject = Buffer<BufferType::Array>;

using IndexBufferObject = Buffer<BufferType::ElementArray>;


/// \brief A VertexArray with a vector of VertexBuffers
/// \deprecated
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



/// \brief Describes the shader parameter aspect of the attribute (location index, value type, normalization)
///
/// This is a description from the Cpp program point of view, more than a shader program introspection.
/// In the sense that is is not concerned with the exact attribute type in the shader, but knows if the data should be normalized.
struct ShaderParameter
{
    enum class Access
    {
        Float,
        Integer,
    };

    // Note: can only normalize for float shader parameters, no need to take the access parameter here.
    /*implicit*/ constexpr ShaderParameter(GLuint aAttributeIndex, bool aNormalize=false) :
        mIndex(aAttributeIndex),
        mNormalize(aNormalize)
    {}

    constexpr ShaderParameter(GLuint aAttributeIndex, Access aAccess) :
        mIndex(aAttributeIndex),
        mTypeInShader(aAccess)
    {}

    GLuint mIndex; // index to match in vertex shader.
    Access mTypeInShader{Access::Float}; // destination data type
    bool mNormalize{false}; // if destination is float and source is integral, should it be normalized (value/type_max_value)
};


/// \brief Describes client perspective of the attribute, i.e. as an argument provided by the client.
struct ClientAttribute
{
    AttributeDimension mDimension;
    size_t mOffset;     // offset for the attribute within the vertex data structure (interleaved)
    GLenum mComponentType;   // data individual components' type.

    constexpr GLsizei sizeBytesFirstDimension() const
    { return mDimension[0] * getByteSize(mComponentType); }

    constexpr GLsizei sizeBytes() const
    { return mDimension[1] * sizeBytesFirstDimension(); }
};


/// \brief The complete description of an attribute as expected by OpenGL.
struct AttributeDescription : public ShaderParameter, ClientAttribute
{};

std::ostream & operator<<(std::ostream &aOut, const AttributeDescription & aDescription);

typedef std::initializer_list<AttributeDescription> AttributeDescriptionList;


/***
 *
 * Vertex Buffer
 *
 ***/

/// \brief Attach currently bound vertex buffer object to currently bound vertex array objet.
/// \param aAttribute Describe the format of the buffer data and the associated parameter in shader program.
void attachBoundVertexBuffer(AttributeDescription aAttribute,
                             GLsizei aStride,
                             GLuint aAttributeDivisor = 0);


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


// TODO Ideally, client-code could invoke the functions expecting std::span 
// without explicitly constructing the std::span{} on call.
/// \brief This overload deduces the stride and size from T_vertex,
/// which could itself be deduced from the provided span.
template <class T_vertex, std::size_t N_spanExtent>
VertexBufferObject loadVertexBuffer(const VertexArrayObject & aVertexArray,
                                    AttributeDescriptionList aAttributes,
                                    std::span<T_vertex, N_spanExtent> aVertices,
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
VertexBufferObject loadUnattachedVertexBuffer(std::span<const T_vertex> aVertices,
                                              GLenum aHint = GL_STATIC_DRAW)
{
    VertexBufferObject vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, aVertices.size_bytes(), aVertices.data(), aHint);
    return vbo;
}


/// \brief High-level function loading a VertexBuffer and directly appending it to a VertexSpecification.
///
/// This is an extension to `loadVertexBuffer()`, which appends the loaded vertex buffer to `aSpecification`.
template <class T_vertex, std::size_t N_spanExtent>
void appendToVertexSpecification(VertexSpecification & aSpecification,
                                 const AttributeDescriptionList & aAttributes,
                                 std::span<T_vertex, N_spanExtent> aVertices,
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
                                  const std::span<T_index> aIndices,
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

    // Note: initially, we passed NULL then set the data following GL wiki:
    // see: https://www.khronos.org/opengl/wiki/Buffer_Object_Streaming
    // Yet, this seems to be counter productive: https://stackoverflow.com/q/31419578/1027706
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, aSize, NULL, GL_STATIC_DRAW);
    //glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, aSize, aData);

    glBufferData(GL_ARRAY_BUFFER, aSize, aData, GL_STATIC_DRAW);
}


/// \brief Overload accepting a span of generic values, instead of low-level void pointer.
/// It works with both vertex and index buffers.
template <class T_values, class T_buffer, std::size_t N_spanExtent>
void respecifyBuffer(const T_buffer & aBufferObject, std::span<T_values, N_spanExtent> aValues)
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
