#include "catch.hpp"


#include <arte/Image.h>

#include <platform/Filesystem.h>

#include <resource/PathProvider.h>

#include <fstream>


using namespace ad;
using namespace ad::arte;


const auto & Red   = math::sdr::gRed;
const auto & Green = math::sdr::gGreen;
const auto & Blue  = math::sdr::gBlue;


filesystem::path ensureTemporaryImageFolder(filesystem::path aSubfolder)
{
    filesystem::path tmp =
#if defined(__APPLE__)
        "/tmp"; // not at all what is returned on macos by temp_directory_path()
#else
        filesystem::temp_directory_path();
#endif

    filesystem::path result =  tmp / aSubfolder;
    create_directories(result);
    return result;
}


template <class T_image>
void requireImagesEquality(const T_image & aLhs, const T_image & aRhs)
{
    REQUIRE(aLhs.width() == aRhs.width());
    REQUIRE(aLhs.height() == aRhs.height());
    REQUIRE(aLhs.size_bytes() == aRhs.size_bytes());
    REQUIRE(std::equal(aLhs.begin(), aLhs.end(), aRhs.begin(), aRhs.end()));
}

SCENARIO("Image type properties")
{
    GIVEN("A default constructed image")
    {
        Image<> def;
        THEN("It is a \"null\" image")
        {
            REQUIRE(def.width() == 0);
            REQUIRE(def.height() == 0);
            REQUIRE(def.size_bytes() == 0);
            REQUIRE(def.data() == nullptr);
        }

    }

    GIVEN("An Image instance")
    {
        Image<> green{ {32, 64}, Green};
        REQUIRE(green.width() == 32);
        REQUIRE(green.height() == 64);
        REQUIRE(green.dimensions().area() == 32*64);
        REQUIRE(green.size_bytes() == 32*64*sizeof(math::sdr::Rgb));
        REQUIRE((green.end() - green.begin()) == green.dimensions().area());

        REQUIRE(std::all_of(green.begin(), green.end(),
                            [&](auto pixel){ return pixel == Green; }));

        THEN("Another image can be copy-constructed from it")
        {
            Image<> copy{green};
            requireImagesEquality(copy, green);
        }

        THEN("It can be assigned to another image (e.g. default constructed)")
        {
            Image<> copy;
            REQUIRE(copy.data() == nullptr);

            copy = green;
            requireImagesEquality(copy, green);
        }

        WHEN("Another image is move-constructed from it")
        {
            const Image<> copy(green);
            Image<> moveTo{std::move(green)};

            THEN("The destination equals the before-move state")
            {
                requireImagesEquality(moveTo, copy);
            }
            THEN("The moved-from image equal the default constructed \"null\" image")
            {
                requireImagesEquality(green, Image<>{});
            }
        }

        WHEN("Another image is move-assigned from it")
        {
            const Image<> copy(green);
            Image<> moveTo{};
            REQUIRE(moveTo.data() == nullptr);
            moveTo = std::move(green);

            THEN("The destination equals the before-move state")
            {
                requireImagesEquality(moveTo, copy);
            }
            THEN("The moved-from image equal the default constructed \"null\" image")
            {
                requireImagesEquality(green, Image<>{});
            }
        }
    }
}


SCENARIO("Image manipulations")
{
    GIVEN("An image alternating red, green, and blue pixels")
    {
        // 5*3 pixels on each dimension
        Image<> alternate( {15, 15}, Red);
        for(auto currentId = 1;
            currentId < alternate.dimensions().area();
            currentId += 3)
        {
            alternate.data()[currentId] = Green;
        }
        for(auto currentId = 2;
            currentId < alternate.dimensions().area();
            currentId += 3)
        {
            alternate.data()[currentId] = Blue;
        }

        THEN("Pixels can be accessed via subscript operator")
        {
            REQUIRE(alternate[0][0] == Red);
            REQUIRE(alternate[1][0] == Green);
            REQUIRE(alternate[2][0] == Blue);

            REQUIRE(alternate[0][1] == Red);
            REQUIRE(alternate[1][1] == Green);
            REQUIRE(alternate[2][1] == Blue);

            REQUIRE(alternate[3][4] == Red);
            REQUIRE(alternate[4][6] == Green);
            REQUIRE(alternate[5][8] == Blue);
        }

        THEN("Subscript operator allows in-place modification")
        {
            REQUIRE(alternate[0][0] == Red);
            alternate[0][0] = Green;
            REQUIRE(alternate[0][0] == Green);
        }

        WHEN("The Image is accessed through constant reference")
        {
            const Image<> & constAlternate = alternate;
            THEN("Pixels can be accessed via subscript operator")
            {
                REQUIRE(constAlternate[0][0] == Red);
                REQUIRE(constAlternate[1][0] == Green);
                REQUIRE(constAlternate[2][0] == Blue);

                REQUIRE(constAlternate[0][1] == Red);
                REQUIRE(constAlternate[1][1] == Green);
                REQUIRE(constAlternate[2][1] == Blue);

                REQUIRE(constAlternate[3][4] == Red);
                REQUIRE(constAlternate[4][6] == Green);
                REQUIRE(constAlternate[5][8] == Blue);

            }

            // TODO why does that compile, when all move and copy are disabled?
            auto theThingThatShouldNotBe = constAlternate[0];
            REQUIRE(!std::is_reference_v<decltype(theThingThatShouldNotBe)>);
        }
    }
}


SCENARIO("Image files creation, read, write")
{
    filesystem::path tempFolder = ensureTemporaryImageFolder("ad_graphics_tests_image");

    GIVEN("Informational message") // Making sure it is just output once
    {
        std::cout << "This test creates images files, "
                << "available for manual inspection in temporary folder: "
                << tempFolder.string()
                << '\n'
                ;
    }

    GIVEN("An image with uniform background color")
    {
        Image<> red{ {512, 512}, math::sdr::gRed };
        THEN("It can be writen to a file")
        {
            auto redfile = tempFolder/"red.ppm";
            red.write(ImageFormat::Ppm,
                      std::ofstream{redfile.string(), std::ios_base::out | std::ios_base::binary});

            REQUIRE(exists(redfile));
        }
    }

    GIVEN("A stream containing a PPM formatted image")
    {
        std::ifstream ppmInput{resource::pathFor("tests/Images/PPM/Yacht.512.ppm").string(),
                               std::ios_base::in | std::ios_base::binary};

        THEN("It can be read to an Image")
        {
            Image<> yacht{Image<>::Read(ImageFormat::Ppm, ppmInput)};

            THEN("Colors can be swapped then it can be written back to a file")
            {
                std::transform(yacht.begin(), yacht.end(), yacht.begin(), [](auto pixel)
                        {
                            auto red = pixel.r();
                            pixel.r() = pixel.b();
                            pixel.b() = pixel.g();
                            pixel.g() = red;
                            return pixel;
                        });

                auto resultfile = tempFolder/"color_swap_yacht.ppm";
                yacht.write(ImageFormat::Ppm,
                            std::ofstream{resultfile.string(),
                                          std::ios_base::out | std::ios_base::binary});
                REQUIRE(exists(resultfile));
            }

            THEN("It can be converted to a grayscale image and written to a file")
            {
                filesystem::path resultfile = tempFolder/"grayscale_yacht.pgm";
                toGrayscale(yacht).saveFile(resultfile);
                REQUIRE(exists(resultfile));
            }
        }
    }
}
