#pragma once


#include <forward_list>
#include <functional>


namespace ad {
namespace handy {


/// \brief A general and simple pool, which keeps a collection of T instances growing on demand.
///
/// The T instances are initialized and configured via a custom `Grower` instance given on Pool construction.
/// The Pool returns smart handles to the T instances via `getNext()`, the handles return the resource to the Pool
/// free list on destruction.
///
/// \important The Poool is not thread safe. This is notably the case of `getNext()` (not too surprising),
/// but also of the SmartHandle destructor (more surprising), because it returns the resource to the Pool's free list.
/// \todo It is not sure that this is a good design decision, because of the implicit invocation of destructors
/// (i.e., they are not called by code, making them notably hard for clients to guard with mutexes).
template <class T>
class Pool
{
private:
    void freeHandle(T * aResource)
    {
        mFreeList.push_front(aResource);
        --mLiveHandles;
    }

public:
    using Grower_t = std::function<T()>;
    using SmartHandle = std::unique_ptr<T, decltype(std::bind(&Pool::freeHandle,
                                                    std::declval<Pool*>(),
                                                    std::placeholders::_1))>;

    Pool(Grower_t aGrower) :
        mGrower{std::move(aGrower)}
    {}

    ~Pool()
    {
        // Otherwise, it means the Pool is destructed while handles are still out there.
        // Note: size() is not available on forward_list
        //assert(mFreeList.size() == mInstances.size());
        assert(mLiveHandles == 0);
    }

    SmartHandle getNext()
    {
        if (mFreeList.empty())
        {
            grow();
        }
        T * free = mFreeList.front();
        mFreeList.pop_front();
        ++mLiveHandles;
        return SmartHandle{free, std::bind(&Pool::freeHandle, this, std::placeholders::_1)};
    }

    void grow()
    {
        // TODO enable logging
        //LOG(handy, debug)("Growing a {} pool", name(T));
        mInstances.push_front(mGrower());
        mFreeList.push_front(&mInstances.front());
    }

private:
    Grower_t mGrower;
    std::forward_list<T> mInstances;
    std::forward_list<T *> mFreeList;
    std::size_t mLiveHandles{0}; // only used for debug assertions, could be macro guarded
};


template <class T>
using Pooled = typename Pool<T>::SmartHandle;
    

} // namespace handy
} // namespace ad
