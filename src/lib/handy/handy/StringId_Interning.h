#pragma once


#include "StringId.h"

#include <mutex>
#include <string>
#include <unordered_map>


namespace ad {
namespace handy {

// Forward declarations for friending et al.
namespace detail {
    class StringIdTableAccess;
}

detail::StringIdTableAccess accessStringIdTable();
StringId internalizeString(const std::string & aString, detail::StringIdTableAccess & aAccess);
std::string revertStringId(StringId);


namespace detail {


    class StringIdTable
    {
        friend StringId ad::handy::internalizeString(const std::string &, detail::StringIdTableAccess &);
        friend StringIdTableAccess ad::handy::accessStringIdTable();
        friend std::string ad::handy::revertStringId(StringId);
        friend class StringIdTableAccess;

    private:
        std::unordered_map<StringId, const std::string> mReverseMap;
        std::mutex mMutex;
    };


    class StringIdTableAccess
    {
        friend StringId ad::handy::internalizeString(const std::string &, detail::StringIdTableAccess &);
        friend StringIdTableAccess ad::handy::accessStringIdTable();
        friend std::string ad::handy::revertStringId(StringId);

    public:
        void clear()
        { mTable.mReverseMap.clear(); }

        std::size_t size()
        { return mTable.mReverseMap.size(); }

    private:
        StringIdTableAccess(StringIdTable & aTable) :
            mTable{aTable},
            mScopedAccess{aTable.mMutex}
        {}

        StringIdTable & mTable;
        std::lock_guard<std::mutex> mScopedAccess;
    };


} // namespace detail


inline detail::StringIdTableAccess accessStringIdTable()
{
    // The global instance
    static detail::StringIdTable table;

    return {table};
}


inline StringId internalizeString(const std::string & aString, detail::StringIdTableAccess & aAccess)
{
    const StringId sid{aString};
    aAccess.mTable.mReverseMap.emplace(sid, aString);
    return sid;
}


inline StringId internalizeString(const std::string & aString)
{
    auto access = accessStringIdTable();
    return internalizeString(aString, access);
}


inline std::string revertStringId(StringId aSid)
{
    return accessStringIdTable().mTable.mReverseMap.at(aSid);
}


} // namespace handy
} // namespace ad
