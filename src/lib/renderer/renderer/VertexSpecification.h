#pragma once

#include "gl_helpers.h"
#include "MappedGL.h"
#include "Range.h"

#include <handy/Guard.h>

#include <gsl/span>

#include <glad/glad.h>

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


/// \brief Describe the attribute access from the shader (layout id, value type, normalization)
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

    GLuint mIndex;
    Access mTypeInShader{Access::Float};
    bool mNormalize{false};
};

/// \brief Attribute and offset of the each entry inside T_element (for interleaved attributes)
template <class T_element, class T_member>
struct AttributeCompact : public Attribute
{
    typedef T_member member_type;

    T_member T_element::* mMember;
};

namespace vertex {

    template <class T_element, class T_member>
    AttributeCompact<T_element, T_member> attr(Attribute aAttribute, T_member T_element::*aMember)
    {
        return {aAttribute, aMember};
    }

} // namespace vertex

/// \brief The complete description of an attribute expected by OpenGL
struct AttributeDescription : public Attribute
{
    GLuint mDimension;
    size_t mOffset;
    GLenum mDataType;
};

std::ostream & operator<<(std::ostream &aOut, const AttributeDescription & aDescription);


template <class T_vertex>
VertexBufferObject loadVertexBuffer(const VertexArrayObject & aVertexArray,
                                    const std::initializer_list<AttributeDescription> & aAttributes,
                                    gsl::span<T_vertex> aVertices)
{
    glBindVertexArray(aVertexArray);
    return makeLoadedVertexBuffer(aAttributes,
                                  sizeof(T_vertex),
                                  sizeof(T_vertex)*aVertices.size(),
                                  aVertices.data());
}


template <class T_vertex>
void appendToVertexSpecification(VertexSpecification & aSpecification,
                                 const std::initializer_list<AttributeDescription> & aAttributes,
                                 gsl::span<T_vertex> aVertices)
{
    aSpecification.mVertexBuffers.push_back(
            loadVertexBuffer(aSpecification.mVertexArray, aAttributes, std::move(aVertices)));
}

/// \brief Create a VertexBufferObject with provided attributes, load it with data.
///
/// This is the lowest level overload, with explicit attribute description and raw data pointer.
/// Other overloads end-up calling it.
VertexBufferObject makeLoadedVertexBuffer(std::initializer_list<AttributeDescription> aAttributes,
                                          GLsizei aStride,
                                          size_t aSize,
                                          const GLvoid * aData);

/// \brief Overload accepting a range instead of a raw pointer.
template <class T_iterator, class T_sentinel>
VertexBufferObject makeLoadedVertexBuffer(std::initializer_list<AttributeDescription> aAttributes,
                                          const Range<T_iterator, T_sentinel> & aRange)
{
    typedef typename Range<T_iterator, T_sentinel>::value_type value_type;

    return makeLoadedVertexBuffer(std::move(aAttributes),
                                  sizeof(value_type),
                                  storedSize(aRange),
                                  data(aRange));
}


/// \brief Overload for a buffer loaded with the data for a **single** attribute.
template <class T_iterator, class T_sentinel>
VertexBufferObject makeLoadedVertexBuffer(Attribute aAttribute,
                                          const Range<T_iterator, T_sentinel>  & aRange)
{
    typedef typename Range<T_iterator, T_sentinel>::value_type value_type;

    AttributeDescription desc{
        aAttribute,
        combined_extents_v<value_type>,
        0,
        MappedGL<scalar_t<value_type>>::enumerator,
    };

    return makeLoadedVertexBuffer({desc},
                                  sizeof(value_type),
                                  storedSize(aRange),
                                  data(aRange));
}

template <class T_object, class T_member>
std::enable_if_t<std::is_standard_layout<T_object>::value, std::size_t>
offset_of(const T_object * aValidObject, T_member T_object::* aMember)
{
    return size_t(&(aValidObject->*aMember)) - size_t(aValidObject);
}

/// \brief High level overload, accepting a range as data source
///        and attributes in their compact form.
template <class T_iterator, class T_sentinel, class... VT_members>
VertexBufferObject makeLoadedVertexBuffer(
        const Range<T_iterator, T_sentinel>  & aRange,
        AttributeCompact<typename Range<T_iterator, T_sentinel>::value_type, VT_members>... vaAttributes)
{
    // Implementer's note:
    //   Ideally, the AttributeCompact would not be templated on the T_member type,
    //   and it would directly store the offset instead.
    //   Yet, this offset computation currently requires an existing instance of T_element
    //   (custom implementation, due to the limitations of offsetof macro,
    //    see: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0908r0.html)
    //   This instance is taken in the provided range, not available at AttributeCompact construction
    //   We are stuck with heterogeneous AttributeCompact types at the moment

    // Because of the need to access first element in the range
    if (size(aRange) == 0)
    {
        throw std::runtime_error(std::string("Unable to invoke ") + __func__ + " on an empty range");
    }

    std::initializer_list<AttributeDescription> attributes = {
        AttributeDescription {
            vaAttributes,
            combined_extents<typename decltype(vaAttributes)::member_type>::value,
            offset_of(&aRange.current(), vaAttributes.mMember),
            MappedGL<scalar_t<typename decltype(vaAttributes)::member_type>>::enumerator
        }...
    };

    return makeLoadedVertexBuffer({attributes}, aRange);
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
inline void respecifyBuffer(const VertexBufferObject & aVBO, const GLvoid * aData)
{
    GLint size;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

    respecifyBuffer(aVBO, aData, size);
}

template <class T_iterator, class T_sentinel>
void respecifyBuffer(const VertexBufferObject & aVBO, Range<T_iterator, T_sentinel> aRange)
{
    respecifyBuffer(aVBO, data(aRange), static_cast<GLsizei>(storedSize(aRange)));
}

} // namespace ad
