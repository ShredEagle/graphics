#pragma once

#include <functional>

class [[nodiscard]] Guard
{
public:
    typedef std::function<void(void)> release_fun;

    Guard(release_fun aReleaser) :
        mReleaser(std::move(aReleaser))
    {}


    // Non-copyable
    Guard (const Guard &) = delete;
    Guard & operator=(const Guard &) = delete;

    // Movable
    Guard (Guard && aOther) :
        mReleaser{std::move(aOther.mReleaser)}
    {
        aOther.mReleaser = &Guard::turnOff;
    }

    Guard & operator=(Guard && aOther)
    {
        mReleaser = std::move(aOther.mReleaser);
        aOther.mReleaser = &Guard::turnOff;
    }

    ~Guard()
    {
        try
        {
            mReleaser();
        }
        catch(...)
        {
            std::cerr << "Catastrophic failure: guard release threw an exception" 
                      << std::endl;
        }
    } 

private:
    static void turnOff()
    {}

private:
    release_fun mReleaser;
};

template <class T>
class [[nodiscard]] ResourceGuard
{
public:
    typedef std::function<void(T &)> release_fun;
    
    ResourceGuard(T aResource, release_fun aReleaser):
        mResource{std::move(aResource)},
        mGuard{std::bind(aReleaser, mResource)}
    {}

    // Movable
    ResourceGuard (ResourceGuard && aOther) :
        mResource{std::move(aOther.mResource)},
        mGuard{std::move(aOther.mGuard)}
    {}

    ResourceGuard & operator=(ResourceGuard &&) = default;

    operator const T& () const
    {
        return mResource;
    }

private:
    T mResource;
    Guard mGuard;
};

template <class T>
ResourceGuard<T> guard(T aResource, typename ResourceGuard<T>::release_fun aReleaser)
{
    return ResourceGuard<T>(std::move(aResource), std::move(aReleaser));
}
