#pragma once

#include "VertexSpecification.h"

#include <handy/array_utils.h>


namespace ad {
namespace graphics {

/// \brief Attribute and offset of the each entry inside T_element (for interleaved attributes)
template <class T_element, class T_member>
struct AttributeCompact : public ShaderParameter
{
    typedef T_member member_type;

    T_member T_element::* mMember;
};

namespace vertex {

    template <class T_element, class T_member>
    AttributeCompact<T_element, T_member> attr(ShaderParameter aAttribute, T_member T_element::*aMember)
    {
        return {aAttribute, aMember};
    }

} // namespace vertex

template <class T_vertex>
VertexBufferObject loadVertexBuffer(
    const VertexArrayObject & aVertexArray,
    ShaderParameter aAttribute,
    const std::span<T_vertex> aVertices)
{
    static_assert(std::is_array<T_vertex>::value || std::is_arithmetic<T_vertex>::value,
                  "Currently only supports spans of arrays or arithmetic types");

    AttributeFormat desc{
        aAttribute,
        combined_extents_v<T_vertex>,
        0,
        MappedGL<scalar_t<T_vertex>>::enumerator,
    };

    return loadVertexBuffer(aVertexArray,
                            {desc},
                            sizeof(T_vertex),
                            aVertices.size_bytes(),
                            aVertices.data(),
                            BufferHint::StaticDraw);
}

template <class T_object, class T_member>
std::enable_if_t<std::is_standard_layout<T_object>::value, std::size_t>
offset_of(const T_object * aValidObject, T_member T_object::* aMember)
{
    return size_t(&(aValidObject->*aMember)) - size_t(aValidObject);
}

/// \brief High level overload, accepting:
/// * a span as data source
/// * attributes in their compact form.
///
/// Note: this has major limitations, the range cannot be empty, the attribute types are limited
/// It is not recommended for use
template <class T_vertex, class... VT_members>
VertexBufferObject makeLoadedVertexBuffer(
        const std::span<T_vertex>  aVertices,
        AttributeCompact<T_vertex, VT_members>... vaAttributes)
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
    if (aVertices.size() == 0)
    {
        throw std::runtime_error(std::string("Unable to invoke ") + __func__ + " on an empty range");
    }

    AttributeDescriptionList attributes = {
        AttributeFormat {
            vaAttributes,
            combined_extents<typename decltype(vaAttributes)::member_type>::value,
            offset_of(&aVertices.front(), vaAttributes.mMember),
            MappedGL<scalar_t<typename decltype(vaAttributes)::member_type>>::enumerator
        }...
    };

    return makeLoadedVertexBuffer({attributes}, aVertices);
}


} // namespace graphics
} // namespace ad
