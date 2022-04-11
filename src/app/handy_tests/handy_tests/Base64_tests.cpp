#include "catch.hpp"

#include <handy/Base64.h>


using namespace ad;
using namespace ad::handy;


SCENARIO("Base64 decoding")
{
    GIVEN("A Base64 encoded string.")
    {
        const std::string sourceData{"Base64 test payload."};
        const std::string encoded{"QmFzZTY0IHRlc3QgcGF5bG9hZC4="}; 

        THEN("It can be decoded back to the original binary.")
        {
            std::vector<std::byte> decoded = base64::decode(encoded);
            std::string decodedToString{reinterpret_cast<char *>(decoded.data()), decoded.size()};
            REQUIRE(decodedToString == sourceData);
            REQUIRE_FALSE(decodedToString == "Non source data");
        }
    }

    GIVEN("A Base64 encoded string.")
    {
        const std::string sourceData{"\0", 1};
        const std::string encoded{"AA=="}; 

        THEN("It can be decoded back to the original binary.")
        {
            std::vector<std::byte> decoded = base64::decode(encoded);
            std::string decodedToString{reinterpret_cast<char *>(decoded.data()), decoded.size()};
            REQUIRE(decodedToString == sourceData);
            REQUIRE_FALSE(decodedToString == "Non source data");
        }
    }

    GIVEN("A Base64 encoded string.")
    {
        const std::string sourceData{"10111120A235rgo lest"};
        const std::string encoded{"MTAxMTExMjBBMjM1cmdvIGxlc3Q="}; 

        THEN("It can be decoded back to the original binary.")
        {
            std::vector<std::byte> decoded = base64::decode(encoded);
            std::string decodedToString{reinterpret_cast<char *>(decoded.data()), decoded.size()};
            REQUIRE(decodedToString == sourceData);
            REQUIRE_FALSE(decodedToString == "Non source data");
        }
    }
}
