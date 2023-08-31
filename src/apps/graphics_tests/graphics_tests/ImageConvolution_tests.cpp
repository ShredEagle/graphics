#include "catch.hpp"

#include "FilesystemHelpers.h"

#include <arte/Image.h>
#include <arte/ImageConvolution.h>

#include <fstream>


using namespace ad;
using namespace ad::arte;


SCENARIO("Image resampling")
{
    filesystem::path tempFolder = ensureTemporaryImageFolder("ad_graphics_tests_image");


    GIVEN("An image")
    {
        ImageRgb yacht{resource::pathFor("tests/Images/PPM/Yacht.512.ppm")};

        THEN("It can be upsampled")
        {
            resampleImage(yacht, {1000, 1000})
                .saveFile(tempFolder / "yacht_upsample.ppm");
        }
    }
}
