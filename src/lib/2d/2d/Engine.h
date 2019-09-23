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

    void appendDraw(const Sprite & aSprite, Position aPosition);

    void clear();
    void render();

    const math::Dimension2<int> & getWindowSize() const;

private:
    DrawContext mDrawContext;
    std::vector<Instance> mSprites;
    math::Dimension2<int> mWindowSize;
};


inline const math::Dimension2<int> & Engine::getWindowSize() const
{
    return mWindowSize;
}

} // namespace ad
