#include "catch.hpp"


#include <handy/StringId.h>
#include <handy/StringId_Interning.h>

#include <string>
#include <unordered_map>


using namespace ad;
using namespace ad::handy;


SCENARIO("StringId literals")
{
    GIVEN("A string id literal")
    {
        using namespace literals;

        constexpr StringId sid = "A string"_sid;

        THEN("It matches another instance on the same string.")
        {
            REQUIRE(StringId("A string") == sid);
        }
        THEN("It does not match another string id.")
        {
            REQUIRE_FALSE(sid == StringId("A_string"));
            REQUIRE_FALSE(sid == StringId("B_string"));
            REQUIRE(sid != StringId("B_string"));
        }
    }
}


SCENARIO("StringId as hash map key.")
{
    using namespace literals;

    GIVEN("An unordered_map whose key is a StringId and the value is the corresponding string.")
    {
        std::unordered_map<StringId, std::string> hashMap;

        WHEN("A few elements are inserted.")
        {
            hashMap.emplace("FirstKey"_sid, "FirstKey");

            const std::string secondKey{"SecondKey"};
            hashMap.emplace(StringId{secondKey}, secondKey);

            THEN("The hash map elements are accessible by their key.")
            {
                CHECK(hashMap.at("FirstKey"_sid) == "FirstKey");
                CHECK_FALSE(hashMap.at("FirstKey"_sid) == secondKey);
                CHECK(hashMap.at(StringId{secondKey}) == secondKey);
            }

            // Check an implementation detail that is not part of the API,
            // but is one of the central motivation for StringId type.
            THEN("The hash used as key is equal to the StringId from the string")
            {
                CHECK(hashMap.find("FirstKey"_sid)->first == "FirstKey"_sid);
                CHECK_FALSE(hashMap.find("FirstKey"_sid)->first == StringId{secondKey});
                CHECK(hashMap.find(StringId{secondKey})->first == StringId{secondKey});
            }
        }
    }
}


SCENARIO("StringId reverse lookup.")
{
    using namespace literals;

    accessStringIdTable().clear();
    REQUIRE(accessStringIdTable().size() == 0);

    GIVEN("A string.")
    {
        const std::string str{"Text content!"};

        THEN("It cannot be found back from the StringId if it was not interned first.")
        {
            REQUIRE_THROWS(revertStringId(StringId{str}));
        }

        WHEN("It is interned.")
        {
            StringId interned = internalizeString(str);

            THEN("The initial string can be found back from the StringId.")
            {
                REQUIRE(revertStringId(interned) == str);
            }
        }

        WHEN("It is interned twice.")
        {
            internalizeString(str);
            internalizeString(str);

            THEN("There is still only one element in the table.")
            {
                REQUIRE(accessStringIdTable().size() == 1);
            }
        }
    }

    GIVEN("Several strings.")
    {
        const std::array<std::string, 3> strings{"un", "deux", "trois"};

        WHEN("They are interned with a single mutex lock/unlock.")
        {
            {
                auto tableAccess = accessStringIdTable();
                for (auto str : strings)
                {
                    internalizeString(str, tableAccess);
                }
            }
            REQUIRE(accessStringIdTable().size() == 3);

            THEN("The initial strings can be found back from the StringId.")
            {
                for (auto str : strings)
                {
                    REQUIRE(revertStringId(StringId{str}) == str);
                }
            }
        }
    }
}


SCENARIO("StringId constexpr-ness")
{
    // It is more a "ensure it compiles" test.
    THEN("Constexpr StringId can be a literal.")
    {
        using namespace literals;
        constexpr StringId sid = "This is a StringId literal"_sid;
    }

    THEN("Constexpr StringId can be constructed from a string literal.")
    {
        constexpr StringId sid{"This is a plain string literal"};
    }
}
