#pragma once


#include <graphics/Texting.h>
#include <graphics/Timer.h>

#include <graphics/AppInterface.h>

#include <platform/Filesystem.h>

// Dirty include, to get the GLFW input definitions
#include <GLFW/glfw3.h>

#include <map>


namespace ad {
namespace graphics {


constexpr GLfloat gScreenWorldHeight = 100;
constexpr GLfloat gGlyphWorldHeight = 10;

class Scene
{
public:
    Scene(filesystem::path aFontPath, std::shared_ptr<AppInterface> aAppInterface) :
        mTexting{
            aFontPath,
            gGlyphWorldHeight,
            gScreenWorldHeight,
            aAppInterface
        }
    {
        mTexting.loadGlyphs(0x20, 0x7F);
    }

    void step(const Timer & aTimer)
    {
    }
    
    void setMessage(const std::string & aMessage)
    {
        std::map<Texture *, std::vector<Texting::Instance>> glyphs;
        mTexting.prepareString(aMessage, {-50.f, 0.f}, glyphs);
        mTexting.updateInstances(glyphs);
    }

    void render()
    {
        mTexting.render();
    }

private:

    void callbackKey(int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
        {
        }
    }

    Texting mTexting;
};


} // namespace graphics
} // namespace ad
