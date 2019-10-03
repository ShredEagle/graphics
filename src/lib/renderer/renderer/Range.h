#pragma once


#include <handy/array_utils.h>


namespace ad {


template <class T>
struct Range
{};


/*********
 * Array *
 *********/
template <class T_element, int N_vertices>
struct Range<T_element[N_vertices]>
{
    static constexpr std::size_t size()
    {
        return N_vertices;
    }

    static constexpr std::size_t getStoredSize()
    {
        return sizeof(mStore);
    }

    static constexpr std::size_t dimension()
    {
        return combined_extents_v<T_element>;
    }

    T_element * data()
    {
        return mStore;
    }

    typedef T_element           element_type;
    typedef scalar_t<T_element> scalar_type;

    T_element (&mStore)[N_vertices];
};


/*****************
 * Iterator pair *
 *****************/
//template <class T_iterator>
//struct Range<std::enable_if>
//{
//    static constexpr std::size_t size()
//    {
//        return std::distance(mFirst, mLast);
//    }
//
//    static constexpr std::size_t getStoredSize()
//    {
//        return size()*sizeof(element_type);
//    }
//
//    static constexpr std::size_t dimension()
//    {
//        return combined_extents_v<element_type>;
//    }
//
//    element_type * data()
//    {
//        return &(*mFirst);
//    }
//
//    typedef std::iterator_traits<T_iterator>::value_type element_type;
//    typedef scalar_t<element_type> scalar_type;
//
//    T_iterator mFirst;
//    T_iterator mLast;
//};

template <class T>
Range<T> range(T & aContainer)
{
    return {aContainer};
}

template <class T_iterator>
Range<T_iterator> range(T_iterator aFirst, T_iterator aLast)
{
    return {aFirst, aLast};
}

template <class T>
auto begin(Range<T> & aRange)
{
    return std::begin(aRange.mStore);
}

template <class T>
auto end(Range<T> & aRange)
{
    return std::end(aRange.mStore);
}

} // namespace ad
