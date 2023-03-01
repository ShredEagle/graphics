#include "Scene.h"

#include <arte/Freetype.h>
#include <arte/Image.h>

#include <freetype/freetype.h>
#include <freetype/ftimage.h>
#include <graphics/ApplicationGlfw.h>
#include <graphics/AppInterface.h>
#include <graphics/Timer.h>

#include <test_commons/PathProvider.h>

#include <memory>

#include <cstring> // for memcopy


// Usage
// Provide a path as first argument to output a dollar sign PGM image there.
// Press enter to switch between message display and font atlas display.
// Press space to toggle kerning.

void bitmapToFile(FT_ULong aCharacterCode, const ad::filesystem::path & aOutputPgm)
{
    ad::arte::Freetype freetype;
    ad::arte::FontFace dejavu = freetype.load(ad::resource::pathFor("fonts/dejavu-fonts-ttf-2.37/DejaVuSans.ttf"));
    dejavu.setPixelHeight(640);
    FT_Bitmap bitmap = dejavu.loadChar(aCharacterCode, FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF))->bitmap;
    std::unique_ptr<unsigned char []> raster{new unsigned char[bitmap.rows * bitmap.width]};
    std::memcpy(raster.get(), bitmap.buffer, bitmap.rows * bitmap.width);

    ad::arte::Image<ad::math::sdr::Grayscale>{
        ad::math::Size<2, int>{static_cast<int>(bitmap.width), static_cast<int>(bitmap.rows)},
        std::move(raster)
    }.saveFile(aOutputPgm);
}

int main(int argc, const char * argv[])
{
    try
    {
        if (argc == 2)
        {
            bitmapToFile(0x24, ad::filesystem::path{argv[1]} / "dollar.pgm");
        }

        ad::graphics::ApplicationGlfw application("Font", 1400, 800);

        ad::graphics::Timer timer{glfwGetTime(), 0.};

        constexpr int glyphPixelHeight = 128;
        constexpr GLfloat screenWorldHeight = 50;
        // cross multiplication to get the glyph world height which maps to the actual pixel height.
        const GLfloat glyphWorldHeight =
            glyphPixelHeight * screenWorldHeight
            / static_cast<float>(application.getAppInterface()->getFramebufferSize().height());
        //ad::filesystem::path relativeFontPath{"fonts/souvenir/souvenirdemiitalic.otf"};
        ad::filesystem::path relativeFontPath{"fonts/dejavu-fonts-ttf-2.37/DejaVuSans.ttf"};
        ad::font::Scene scene{ad::resource::pathFor(relativeFontPath),
                              glyphPixelHeight,
                              glyphWorldHeight,
                              screenWorldHeight,
                              *application.getAppInterface()};

        while(application.nextFrame())
        {
            application.getAppInterface()->clear();
            scene.step(timer);
            scene.render();
            timer.mark(glfwGetTime());
        }
    }
    catch(const std::exception & e)
    {
        std::cerr << "Exception:\n"
                  << e.what()
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::exit(EXIT_SUCCESS);
}
