#include "catch.hpp"

#include <renderer/Shading.h>

#include <test_commons/PathProvider.h>

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


SCENARIO("Shader code (std::string) include preprocessing.")
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


SCENARIO("Shader files include preprocessing.")
{
    GIVEN("The path to a top-level shader including other files.")
    {
        std::filesystem::path top = resource::pathFor("tests/Shaders/a.glsl");

        WHEN("It is preprocessed.")
        {
            ShaderSource source = ShaderSource::Preprocess(top);

            THEN("The resulting amalgamation includes the other strings.")
            {
                const std::string expected{"int a;\nint sub;\nint subsub;\nint b;"};
                ShaderSourceView view{source};
                REQUIRE(view.mSource == expected);
            }
        }
    }
}
