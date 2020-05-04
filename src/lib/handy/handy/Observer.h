#pragma once

#include <list>


namespace ad {


/// \brief Simple Obsever pattern implementation, based on weak references to observers.
/// \note To remove an observers, invalidate all shared_pointers to it.
template <class T_observer>
struct Subject
{
    template <class... VT_aArgs>
    void dispatch(VT_aArgs &&... aArgs);

    std::list<std::weak_ptr<T_observer>> mObservers;
};


template <class T_observer>
template <class... VT_aArgs>
void Subject<T_observer>::dispatch(VT_aArgs &&... aArgs)
{
    for(auto current = mObservers.begin(); current != mObservers.end(); /*increment in body*/)
    {
        if (auto observer = current->lock())
        {
            (*observer)(std::forward<VT_aArgs>(aArgs)...);
            ++current;
        }
        else
        {
            current = mObservers.erase(current);
        }
    }
}


} // namespace ad
