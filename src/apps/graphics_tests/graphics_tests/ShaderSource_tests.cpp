#include "catch.hpp"

#include <renderer/Shading.h>

#include <map>


using namespace ad;
using namespace ad::graphics;


const std::string gShaderA = 
R"(#version 310

//#include "ShaderC"
#include "ShaderB" //#include"ShaderC"

int a; #include "ShaderC"

void main(void)
{
})";


const std::string gShaderB = 
R"(int b;//startb
int foo()
{}
// endb)";


const std::string gShaderC = 
R"(int c;)";


const std::string gExpectedAmalgamation =
R"(#version 310


int b;
int foo()
{}
 

int a; int c;

void main(void)
{
})";


std::unique_ptr<std::istringstream> lookupStringTable(const std::string aStringName)
{
    static const std::map<std::string, const std::string *> lookupTable{
        {"ShaderA", &gShaderA},
        {"ShaderB", &gShaderB},
        {"ShaderC", &gShaderC},
    };

    return std::make_unique<std::istringstream>(*lookupTable.at(aStringName));
}


SCENARIO("Shader code include preprocessing.")
{
    GIVEN("A shader source with #include statements")
    {
        // gShaderA;

        WHEN("It is preprocessed.")
        {
            ShaderSource source = ShaderSource::Preprocess(
                std::istringstream{gShaderA},
                lookupStringTable);

            THEN("The resulting amalgamation includes the other strings.")
            {
                ShaderSourceView view{source};
                REQUIRE(view.mSource == gExpectedAmalgamation);
            }
        }
    }
}
