#pragma once


#include <type_traits>


// TODO Ad 31/01/2022: This approach has semantic issues.
// Most notably, the bitwise operators might create enumerators that are not present
// is the scoped enum.
// There should probably be a strongly typed distinction between the base enumerators
// and a value constructed by combining those enumerators.
// (E.g. The enumerator could be used as positions for a std::bitset, wich would be the actual value).


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
T_enumClass & operator|=(T_enumClass & aLhs, T_enumClass aRhs)
{
    aLhs = aLhs | aRhs;
    return aLhs;
}


template <class T_enumClass>
requires is_bitmask_v<T_enumClass>
T_enumClass operator&(T_enumClass aLhs, T_enumClass aRhs)
{
    return static_cast<T_enumClass>(
        static_cast<std::underlying_type_t<T_enumClass>>(aLhs)
        & static_cast<std::underlying_type_t<T_enumClass>>(aRhs));
}


template <class T_enumClass>
requires is_bitmask_v<T_enumClass>
T_enumClass & operator&=(T_enumClass & aLhs, T_enumClass aRhs)
{
    aLhs = aLhs & aRhs;
    return aLhs;
}


template <class T_enumClass>
requires is_bitmask_v<T_enumClass>
bool test(T_enumClass aFlags, T_enumClass aTested)
{
    return (aFlags & aTested) == aTested;
}


} // namespace ad
