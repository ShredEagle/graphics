#pragma once

#include <type_traits>


namespace ad {


namespace detail
{

    template <class T, unsigned N = std::rank<T>::value>
    struct combined_extents_impl :
        public std::integral_constant<std::size_t,
                                      std::extent<T, N-1>::value * combined_extents_impl<T, N-1>::value>
    {};

    template <class T>
    struct combined_extents_impl<T, 0> : public std::integral_constant<std::size_t, 1>
    {};

} // namespace detail

/// \brief Combines all dimensions in an array (the total number of elements stored)
/// \return 1 for scalar types !
template <class T>
struct combined_extents : public detail::combined_extents_impl<T>
{};

template <class T>
static constexpr std::size_t combined_extents_v = combined_extents<T>::value;


template <class T>
struct scalar
{
    typedef T type;
};

template <class T>
struct scalar<T[]>
{
    typedef typename scalar<T>::type type;
};

template <class T, std::size_t N>
struct scalar<T[N]>
{
    typedef typename scalar<T>::type type;
};

//template <class T>
//using scalar_t = std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>;

template <class T>
using scalar_t = typename scalar<T>::type;

} // namespace ad
