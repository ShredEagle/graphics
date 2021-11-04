#include "Freetype.h"
#include "Scene.h"

#include <arte/Image.h>

#include <graphics/ApplicationGlfw.h>
#include <graphics/AppInterface.h>
#include <graphics/Timer.h>

#include <resource/PathProvider.h>

#include <memory>


// Usage
// Provide a path as first argument to output a dollar sign PGM image there.
// Press enter to switch between message display and font atlas display.

void bitmapToFile(FT_ULong aCharacterCode, const ad::filesystem::path & aOutputPgm)
{
    ad::graphics::Freetype freetype;
    FontFace dejavu = freetype.load(ad::resource::pathFor("fonts/dejavu-fonts-ttf-2.37/DejaVuSans.ttf"));
    dejavu.setPixelHeight(640);
    GlyphBitmap bitmap = dejavu.getGlyph(aCharacterCode).render();
    std::unique_ptr<char []> raster{new char[bitmap.bytesize()]};
    std::memcpy(raster.get(), bitmap.data(), bitmap.bytesize());

    ad::arte::Image<ad::math::sdr::Grayscale>{ {bitmap.width(), bitmap.rows()}, std::move(raster)}
        .saveFile(aOutputPgm);
}

int main(int argc, const char * argv[])
{
    try
    {
        if (argc == 2)
        {
            bitmapToFile(0x24, ad::filesystem::path{argv[1]} / "dollar.pgm");
        }

        ad::graphics::ApplicationGlfw application("Font", 800, 600);

        ad::graphics::Timer timer{glfwGetTime(), 0.};

        constexpr int glyphPixelHeight = 256;
        // TODO exact pixel
        constexpr GLfloat glyphWorldHeight = 10;
        constexpr GLfloat screenWorldHeight = 100;
        ad::font::Scene scene{ad::resource::pathFor("fonts/dejavu-fonts-ttf-2.37/DejaVuSans.ttf"), 
                              glyphPixelHeight, glyphWorldHeight,
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
