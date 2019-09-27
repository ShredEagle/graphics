#pragma once

#include "commons.h"
#include "Sprite.h"

#include <renderer/Drawing.h>

namespace ad {

struct Instance
{
    Instance(Position2<GLint> aPosition, Rectangle<GLint> aTextureArea):
        mPosition(aPosition),
        mTextureArea(aTextureArea)
    {}

    Position2<GLint> mPosition;
    Rectangle<GLint> mTextureArea;
};

const std::vector<const AttributeDescription> gInstanceDescription = {
    {2, 2, offsetof(Instance, mPosition),    MappedGL<GLint>::enumerator},
    {3, 4, offsetof(Instance, mTextureArea), MappedGL<GLint>::enumerator, ShaderAccess::Integer},
};

class Engine
{
public:
    Engine();

    void callbackWindowSize(int width, int height);
    std::vector<Sprite> loadSheet(const std::string &aPath);

    void appendDraw(const Sprite & aSprite, Position2<GLint> aPosition);

    void clear();
    void render();

    const Size2<int> & getWindowSize() const;

private:
    DrawContext mDrawContext;
    std::vector<Instance> mSprites;
    Size2<int> mWindowSize;
};


inline const Size2<int> & Engine::getWindowSize() const
{
    return mWindowSize;
}

} // namespace ad
