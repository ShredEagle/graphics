#pragma once


#include <graphics/Texting.h>
#include <graphics/Timer.h>

#include <graphics/AppInterface.h>

#include <platform/Filesystem.h>

// Dirty include, to get the GLFW input definitions
#include <GLFW/glfw3.h>

#include <functional>
#include <map>


namespace ad {
namespace graphics {


constexpr GLfloat gScreenWorldHeight = 1000;

class Scene
{
public:
    Scene(filesystem::path aFontPath, std::shared_ptr<AppInterface> aAppInterface, GLfloat aGlyphWorldHeight) :
        mTexting{
            aFontPath,
            aGlyphWorldHeight,
            gScreenWorldHeight,
            aAppInterface
        }
    {
        using namespace std::placeholders;
        aAppInterface->registerKeyCallback(std::bind(&Scene::onKey, this, _1, _2, _3, _4));
        mTexting.loadGlyphs(0x20, 0x7F);
    }

    void step(const Timer & aTimer)
    {
        std::map<Texture *, std::vector<Texting::Instance>> glyphs;
        mTexting.prepareString(mMessage, mPenPosition, math::sdr::gCyan, glyphs);
        mTexting.updateInstances(glyphs);
    }
    
    void setMessage(const std::string & aMessage)
    {
        mMessage = aMessage;
    }

    void render()
    {
        mTexting.render();
    }

private:

    void onKey(int key, int scancode, int action, int mods)
    {
        constexpr double step = 0.25f;
        if (key == GLFW_KEY_UP && action == GLFW_PRESS)
        {
            mPenPosition.y() += step;
        }
        else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
        {
            mPenPosition.y() -= step;
        }
        else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
        {
            mPenPosition.x() += step;
        }
        else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
        {
            mPenPosition.x() -= step;
        }
    }

    Texting mTexting;
    std::string mMessage;
    math::Position<2, GLfloat> mPenPosition{-50.f, 0.f};
};


} // namespace graphics
} // namespace ad
