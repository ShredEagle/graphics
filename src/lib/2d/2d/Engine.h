#pragma once

#include "commons.h"
#include "Sprite.h"

#include <renderer/Drawing.h>

namespace ad {

struct Instance
{
    Instance(Position aPosition, Rectangle aTextureArea):
        mPosition(aPosition),
        mTextureArea(aTextureArea)
    {}

    Position mPosition;
    Rectangle mTextureArea;
};

class Engine
{
public:
    Engine();

    void callbackWindowSize(int width, int height);
    std::vector<Sprite> loadSheet(const std::string &aPath);

    void appendDraw(const Sprite & aSprite, Position aPosition);

    void render();

private:
    DrawContext mDrawContext;
    std::vector<Instance> mSprites;
    math::Dimension2<int> mWindowSize;
};

} // namespace ad
