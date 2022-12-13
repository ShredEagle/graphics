#include "catch.hpp"

#include <renderer/ShaderSource.h>
#include <renderer/Shading.h>
#include <graphics/ApplicationGlfw.h>

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
}
)";


std::pair<std::unique_ptr<std::istringstream>, std::string>
lookupStringTable(const std::string aStringName)
{
    static const std::map<std::string, const std::string *> lookupTable{
        {"ShaderA", &gShaderA},
        {"ShaderB", &gShaderB},
        {"ShaderC", &gShaderC},
    };

    return {
        std::make_unique<std::istringstream>(*lookupTable.at(aStringName)),
        aStringName,
    };
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
                "ShaderA",
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
        std::filesystem::path top = resource::pathFor("tests/Shaders/nested_inclusion/a.glsl");
        std::filesystem::path base = top.parent_path();

        WHEN("It is preprocessed.")
        {
            ShaderSource source = ShaderSource::Preprocess(top);

            THEN("The resulting amalgamation includes the other strings.")
            {
                // Note: the blank line before `int a_after` is discarded.
                const std::string expected{
                    "int a_before;\nint sub;\nint subsub;\nint b1;\nint b2;\nint a_after;\n"
                };
                ShaderSourceView view{source};
                REQUIRE(view.mSource == expected);
            }

            THEN("The source map is allowing to map each line in the output to original files.")
            {
                const auto & sourceMap = source.getSourceMap();

                auto mapping = sourceMap.getLine(1);
                CHECK(mapping.mIdentifier == top);
                CHECK(mapping.mLine == 1);

                mapping = sourceMap.getLine(2);
                CHECK(mapping.mIdentifier == base / "sub" / "sub.glsl");
                CHECK(mapping.mLine == 1);

                mapping = sourceMap.getLine(3);
                CHECK(mapping.mIdentifier == base / "sub" / "subsub" / "subsub.glsl");
                CHECK(mapping.mLine == 1);

                mapping = sourceMap.getLine(4);
                CHECK(mapping.mIdentifier == base / "b.glsl");
                CHECK(mapping.mLine == 1);

                mapping = sourceMap.getLine(5);
                CHECK(mapping.mIdentifier == base / "b.glsl");
                CHECK(mapping.mLine == 2);

                // Note: Yet the blank line is counted when mapping to original source
                mapping = sourceMap.getLine(6);
                CHECK(mapping.mIdentifier == base / "a.glsl");
                CHECK(mapping.mLine == 5);
            }
        }
    }
}


SCENARIO("Shader compilation errors mapping.")
{
    if (GLAD_GL_VERSION_3_1)
    {
        GIVEN("A preprocessed shader file including another-one containing an error.")
        {
            std::filesystem::path vert = resource::pathFor("tests/Shaders/compilation_errors/vert.glsl");
            ShaderSource source = ShaderSource::Preprocess(vert);

            WHEN("It is compiled.")
            {
                graphics::ApplicationGlfw app{"dummy", {1, 1}};

                graphics::Shader shader{GL_VERTEX_SHADER};
                THEN("It throws an exception pointing to the correct line in the base files.")
                {
                    using Catch::Matchers::Contains;
                    CHECK_THROWS_WITH(compileShader(shader, source),
                                       Contains("helpers.glsl 0(line: 3)")
                                         && Contains("vert.glsl 0(line: 7)"));
                }
            }
        }
    }
}
