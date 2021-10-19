#pragma once

#include <tuple>
#include <type_traits>

namespace ad {

template <size_t I=0, class T_func, class ...VT_args>
typename std::enable_if<I != sizeof...(VT_args)>::type
for_each(std::tuple<VT_args...>& aTuple, T_func operation)
{
    operation(std::get<I>(aTuple));
    for_each<I+1>(aTuple, operation);
}

template <size_t I=0, class T_func, class ...VT_args>
typename std::enable_if<I == sizeof...(VT_args)>::type
for_each(std::tuple<VT_args...>&, T_func)
{}

} // namespace ad
