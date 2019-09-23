#pragma once

#include "commons.h"

#include "Sprite.h"

#include <renderer/Drawing.h>

namespace ad {

class Engine;

class Tiling
{
    /// \todo make const so the size is fixed to the gride size
    typedef std::vector<Color> instance_data;
public:
    Tiling(Size2<int> aCellSize, Size2<int> aGridDefinition);

    std::vector<LoadedSprite> load(const SpriteSheet &aSpriteSheet);

    instance_data::iterator begin(); 
    instance_data::iterator end(); 

    /// \note Does it make sense to forward engine here?
    ///       What is the real meaning of the Engine class
    void render(const Engine & aEngine) const;

private:
    instance_data mColors;
    DrawContext mDrawContext;
    //std::vector<Rectangle> mTiles;

    static constexpr GLsizei gVerticesPerInstance{4};
};

} // namespace ad
