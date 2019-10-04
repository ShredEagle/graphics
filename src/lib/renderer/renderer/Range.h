#pragma once


#include <handy/array_utils.h>

#include <vector>


namespace ad {


// An Q&D implementation of contiguous iterator test,
// since `contiguous_iterator_tag` is not present until C++20
namespace detail
{

    // Hats-off to MZH:
    // https://what.thedailywtf.com/topic/23173/contiguous-iterators-c

    // Only the existence of the function is of interest
    // (if the function is detected for a type T, then T will be considered a contiguous iterator)
    template <class T>
    constexpr auto is_contiguous() ->
        std::enable_if_t<
            std::is_same<T,
                         typename std::vector<typename std::iterator_traits<T>::value_type>::iterator
                        >::value,
            void>;

    template <class T>
    constexpr auto is_contiguous() ->
        std::enable_if_t<
            std::is_same<T,
                         typename std::vector<typename std::iterator_traits<T>::value_type>::const_iterator
                        >::value,
            void>;

    template <class T, class AlwaysVoid>
    struct is_contiguousiterator : public std::false_type
    {};

    template <class T>
    struct is_contiguousiterator<T, decltype(is_contiguous<T>())> : public std::true_type
    {};

    template <class T>
    struct is_contiguousiterator<T*, void> : public std::true_type
    {};

} // namespace detail


/// \brief Conventional access to the contiguous iterator test value
template <class T>
static constexpr bool is_contiguousiterator_v = detail::is_contiguousiterator<T, void>::value;


    /*********
     * Range *
     *********/

/// \brief Naive range implementation, waiting for inclusion of range-v3
///        or C++20 std ranges
///
/// For importance of allowing a different sentinel type, see:
/// http://ericniebler.com/2014/02/
template <class T_iterator, class T_sentinel=T_iterator>
struct Range
{
    typedef typename std::iterator_traits<T_iterator>::reference  reference;
    typedef typename std::iterator_traits<T_iterator>::value_type value_type;

    Range(T_iterator aIterator, T_sentinel aSentinel) :
        mCurrent(aIterator),
        mSentinel(aSentinel)
    {}

    reference current() const
    {
        return *mCurrent;
    }

    void next()
    {
        ++mCurrent;
    }

    bool done() const
    {
        return mCurrent == mSentinel;
    }

/// \todo privatize
    T_iterator mCurrent;
    T_sentinel mSentinel;
};


/*
 * Range helper functions
 */
template <class T_iterator, class T_sentinel>
std::enable_if_t<is_contiguousiterator_v<T_iterator>,
                 const typename Range<T_iterator, T_sentinel>::value_type * >
data(const Range<T_iterator, T_sentinel> &aRange)
{
    return &(*aRange.mCurrent);
}

template <class T_iterator>
std::size_t size(const Range<T_iterator, T_iterator> &aRange)
{
    return std::distance(aRange.mCurrent, aRange.mSentinel);
}

template <class T_iterator>
std::size_t storedSize(const Range<T_iterator, T_iterator> &aRange)
{
    return size(aRange)*sizeof(typename std::remove_reference_t<decltype(aRange)>::value_type);
}


/*
 * Range factory functions
 */
template <class T_container>
auto range(T_container & aContainer)
{
    return Range<decltype(std::begin(std::declval<T_container&>()))>
                {std::begin(aContainer), std::end(aContainer)};
}

template <class T_iterator>
auto range(T_iterator aFirst, T_iterator aLast)
{
    return Range<T_iterator>{std::move(aFirst), std::move(aLast)};
}

} // namespace ad
