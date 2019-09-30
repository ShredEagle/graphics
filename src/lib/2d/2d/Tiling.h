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
    /// \todo Would be better with a container of const size
    typedef instance_data::iterator iterator;

    Tiling(Size2<int> aCellSize, Size2<int> aGridDefinition, Size2<int> aRenderResolution);

    void resetTiling(Size2<int> aCellSize, Size2<int> aGridDefinition);

    /// \brief Takes a pair of iterator to SpriteArea instances, and the corresponding raster data
    template <class T_iterator>
    std::vector<LoadedSprite> load(T_iterator aFirst, T_iterator aLast,
                                   const Image & aRasterData);

    iterator begin();
    iterator end();

    void setBufferResolution(Size2<int> aNewResolution);

    /// \note Does it make sense to forward engine here?
    ///       What is the real meaning of the Engine class
    void render(const Engine & aEngine) const;

    Position2<GLint> getPosition() const;
    void setPosition(Position2<GLint> aPosition);

    Rectangle<GLint> getGridRectangle() const;

    Size2<GLint> getTileSize() const;
    Size2<GLint> getGridDefinition() const;

private:
    DrawContext mDrawContext;
    //instance_data mColors;
    instance_data mTiles;

    Size2<GLint> mTileSize;
    Size2<GLint> mGridDefinition;
    Rectangle<GLint> mGridRectangleScreen;

    static constexpr GLsizei gVerticesPerInstance{4};
};


/*
 * Implementations
 */
inline Position2<GLint> Tiling::getPosition() const
{
    return mGridRectangleScreen.mPosition;
}

inline Rectangle<GLint> Tiling::getGridRectangle() const
{
    return mGridRectangleScreen;
}

inline Size2<GLint> Tiling::getTileSize() const
{
    return mTileSize;
}

inline Size2<GLint> Tiling::getGridDefinition() const
{
    return mGridDefinition;
}

} // namespace ad

#include "Tiling-impl.h"

