#pragma once

#include "gl_helpers.h"
#include "MappedGL.h"
#include "Range.h"

#include <handy/Guard.h>

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

template <class T_element, class T_member>
struct AttributeCompact : public Attribute
{
    typedef T_member member_type;

    T_member T_element::* mMember;
};

struct AttributeDescription : public Attribute
{
    GLuint mDimension;
    size_t mOffset;
    GLenum mDataType;
};

inline std::ostream & operator<<(std::ostream &aOut, const AttributeDescription & aDescription)
{
    return aOut << "Index " << aDescription.mIndex << " | "
                << "Dimension " << aDescription.mDimension << " | "
                << "Offset " << aDescription.mOffset << " | "
                << "Data " << aDescription.mDataType
                ;
}


/// \todo To the impl file?
inline VertexBufferObject makeLoadedVertexBuffer(std::initializer_list<AttributeDescription> aAttributes,
                                                 GLsizei aStride,
                                                 size_t aSize,
                                                 const GLvoid * aData)
{
    /// \todo some static assert on the PODness of the element type

    VertexBufferObject vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, aSize, aData, GL_STATIC_DRAW);

    for (const auto & attribute : aAttributes)
    {
        switch(attribute.mTypeInShader)
        {
            case Attribute::Access::Float :
            {
                glVertexAttribPointer(attribute.mIndex,
                                      attribute.mDimension,
                                      attribute.mDataType,
                                      attribute.mNormalize,
                                      aStride,
                                      reinterpret_cast<const void*>(attribute.mOffset));
                break;
            }
            case Attribute::Access::Integer :
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
VertexBufferObject makeLoadedVertexBuffer(std::initializer_list<AttributeDescription> aAttributes,
                                          typename std::vector<T_element>::const_iterator aFirst,
                                          typename std::vector<T_element>::const_iterator aLast)
{
    return makeLoadedVertexBuffer(std::move(aAttributes),
                                  sizeof(T_element),
                                  sizeof(T_element) * std::distance(aFirst, aLast),
                                  &(*aFirst));
}


template <class T_rangeElement>
VertexBufferObject makeLoadedVertexBuffer(Attribute aAttribute, Range<T_rangeElement> aRange)
{
    AttributeDescription desc{
        aAttribute,
        aRange.dimension(),
        0,
        MappedGL<typename decltype(aRange)::scalar_type>::enumerator,
    };

    return makeLoadedVertexBuffer({desc},
                                  sizeof(typename decltype(aRange)::element_type),
                                  aRange.getStoredSize(),
                                  aRange.data());
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


template <class T_iterator>
void respecifyBuffer(const VertexBufferObject & aVBO, T_iterator aFirst, T_iterator aLast)
{
    respecifyBuffer(aVBO,
                    &(*aFirst),
                    sizeof(typename T_iterator::value_type)*std::distance(aFirst, aLast));
}


/// \brief Respecify the buffer with the same size (allowing potential optimizations)
template <class T_data>
void respecifyBuffer(const VertexBufferObject & aVBO, const T_data & aData)
{
    GLint size;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

    respecifyBuffer(aVBO, aData, size);
}


////


template <class T_object, class T_member>
std::enable_if_t<std::is_standard_layout<T_object>::value, std::size_t>
offset_of(const T_object * aValidObject, T_member T_object::* aMember)
{
    return size_t(&(aValidObject->*aMember)) - size_t(aValidObject);
}

template <class T_element, class T_member>
AttributeCompact<T_element, T_member> attr(GLuint aIndex, T_member T_element::*aMember)
{
    return { {aIndex}, aMember};
}

template <class T_element, class... VT_members>
VertexBufferObject makeLoadedVertexBuffer(const std::vector<T_element> & aRange,
                                          AttributeCompact<T_element, VT_members>... vaAttributes)
{
    /// \todo Early return on empty range (we use the first element in deduction)

    /// \todo Provide a way to customise shader access (float, normalize, ...)
    std::initializer_list<AttributeDescription> attributes = {
        AttributeDescription {
            {vaAttributes.mIndex},
            combined_extents<typename decltype(vaAttributes)::member_type>::value,
            offset_of(&aRange.front(), vaAttributes.mMember),
            MappedGL<scalar_t<typename decltype(vaAttributes)::member_type>>::enumerator
        }...
    };

    //for (auto attr : attributes)
    //{
    //    std::cout << "OFFI: " << attr.mIndex << " " << attr.mDimension << " " << attr.mOffset << std::endl;
    //}
    return makeLoadedVertexBuffer<T_element>({attributes}, aRange.begin(), aRange.end());
}

} // namespace ad
