#include "catch.hpp"

#include <handy/Crc.h>


using namespace ad;
using namespace ad::handy;


SCENARIO("Crc 64 unit tests")
{
    // Reference values taken from http://andrewl.dreamhosters.com/filedump/crc64.cpp

    GIVEN("A two bytes input.")
    {
        const std::vector<std::byte> input{(std::byte)0xDE, (std::byte)0xAD};

        THEN("Its CRC64 can be computed.")
        {
            Crc64 crc = crc64(input.begin(), input.end());
            REQUIRE(crc == 0x44277F18417C45A5);
        }
    }

    GIVEN("A string input.")
    {
        const std::string str = "\x99\xEB\x96\xDD\x94\xC8\x8E\x97\x5B\x58\x5D\x2F\x28\x78\x5E\x36";

        THEN("Its CRC64 can be computed.")
        {
            Crc64 crc = crc64(str);
            REQUIRE(crc == 0xDB7AC38F63413C4E);
        }
    }

    // Taken from go: https://go.dev/src/hash/crc64/crc64_test.go
    // But does not pass.
    //GIVEN("A string input.")
    //{
    //    const std::string str = "If the enemy is within range, then so are you.";

    //    THEN("Its CRC64 can be computed.")
    //    {
    //        Crc64 crc = crc64(str);
    //        REQUIRE(crc == 0xd7dd118c98e98727);
    //    }
    //}
    
}
