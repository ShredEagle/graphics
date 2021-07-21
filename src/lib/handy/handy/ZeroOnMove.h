#pragma once

#include <type_traits>
#include <utility>


namespace ad {


// \brief A zero-memory overhead wrapper around T, resetting value to zero when moved-from.
//
// Has implicit conversions to and from T.
template <class T>
class ZeroOnMove
{
    static_assert(std::is_arithmetic<T>::value,
                  "Base template only intended for arithmetic types.");

public:
    ZeroOnMove() noexcept = default;

    /*implicit*/ ZeroOnMove(T aValue) noexcept :
        mValue(std::move(aValue))
    {}

    /*implicit*/ operator T & ()
    {
        return mValue;
    }

    /*implicit*/ operator T() const
    {
        return mValue;
    }

    ZeroOnMove(const ZeroOnMove & aRhs) noexcept = default;
    ZeroOnMove & operator=(const ZeroOnMove & aRhs) noexcept = default;

    ZeroOnMove(ZeroOnMove && aRhs) noexcept :
        mValue(aRhs.mValue)
    {
        aRhs.mValue = 0;
    }

    ZeroOnMove & operator=(ZeroOnMove && aRhs) noexcept
    {
        mValue = aRhs.mValue;
        aRhs.mValue = 0;
        return *this;
    }

private:
    T mValue{0};
};


} // namespace ad
