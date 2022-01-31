#pragma once


#include <type_traits>


// Note: putting it in handy subnamespace would make the operators difficult to use.
namespace ad {


template <class>
struct is_bitmask : public std::false_type
{};


template <class T>
constexpr bool is_bitmask_v = is_bitmask<T>::value;


template <class T_enumClass>
requires is_bitmask_v<T_enumClass>
T_enumClass operator|(T_enumClass aLhs, T_enumClass aRhs)
{
    return static_cast<T_enumClass>(
        static_cast<std::underlying_type_t<T_enumClass>>(aLhs)
        | static_cast<std::underlying_type_t<T_enumClass>>(aRhs));
}


template <class T_enumClass>
requires is_bitmask_v<T_enumClass>
T_enumClass operator&(T_enumClass aLhs, T_enumClass aRhs)
{
    return static_cast<T_enumClass>(
        static_cast<std::underlying_type_t<T_enumClass>>(aLhs)
        & static_cast<std::underlying_type_t<T_enumClass>>(aRhs));
}


} // namespace ad
