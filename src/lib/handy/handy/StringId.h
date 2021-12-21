#pragma once


#include "Crc.h"


namespace ad {
namespace handy {


class StringId
{
    friend std::hash<StringId>;

    // Only usefull for Null static instance
    constexpr StringId();

public:
    explicit constexpr StringId(std::string_view aStringView);
    explicit constexpr StringId(const char * aChars, std::size_t aCount);

    constexpr bool operator==(StringId aRhs) const;
    constexpr bool operator!=(StringId aRhs) const;

    // To be usable in std::map
    constexpr bool operator<(StringId aRhs) const;

    static constexpr StringId Null()
    { return StringId{}; }

private:
    Crc64 mUid;
};


namespace literals {

    constexpr StringId operator ""_sid(const char * aChars, std::size_t aCount)
    {
        return StringId(aChars, aCount);
    }

} // namespace literals


//
// Implementations
//
inline constexpr StringId::StringId(std::string_view aStringView) :
    mUid{crc64(aStringView)}
{}


inline constexpr StringId::StringId(const char * aChars, std::size_t aCount) :
    mUid{crc64(aChars, aChars + aCount)}
{}


constexpr StringId::StringId() :
    mUid{0}
{}


inline constexpr bool StringId::operator==(StringId aRhs) const
{
    return mUid == aRhs.mUid;
}


inline constexpr bool StringId::operator!=(StringId aRhs) const
{
    return !(*this == aRhs);
}


inline constexpr bool StringId::operator<(StringId aRhs) const
{
    return mUid < aRhs.mUid;
}


} // namespace handy
} // namespace ad

//
// Specialization of std::hash for StringId
//
namespace std {

template<> struct hash<ad::handy::StringId>
{
    constexpr size_t operator()(const ad::handy::StringId & aStringId) const
    {
        return aStringId.mUid;
    }
};

} // namespace std
