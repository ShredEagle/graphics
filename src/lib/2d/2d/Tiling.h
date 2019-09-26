#pragma once

#include "commons.h"

#include "Sprite.h"

#include <renderer/Drawing.h>

namespace ad {

class Engine;

class Tiling
{
    /// \todo Would be better with a container of const size
    typedef std::vector<LoadedSprite> instance_data;

public:
    Tiling(Size2<int> aCellSize, Size2<int> aGridDefinition);

    /// \brief Takes a pair of iterator to SpriteArea instances, and the corresponding raster data
    template <class T_iterator>
    std::vector<LoadedSprite> load(T_iterator aFirst, T_iterator aLast,
                                   const Image & aRasterData);

    instance_data::iterator begin(); 
    instance_data::iterator end(); 

    /// \note Does it make sense to forward engine here?
    ///       What is the real meaning of the Engine class
    void render(const Engine & aEngine) const;

private:
    DrawContext mDrawContext;
    //instance_data mColors;
    instance_data mTiles;

    static constexpr GLsizei gVerticesPerInstance{4};
};

} // namespace ad

#include "Tiling-impl.h"

