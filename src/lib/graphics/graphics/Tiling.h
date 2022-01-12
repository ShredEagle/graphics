#pragma once

#include "commons.h"

#include "Sprite.h"

#include <renderer/Drawing.h>

namespace ad {
namespace graphics{


class AppInterface;


class Tiling
{
    /// \todo Would be better with a container of const size
    typedef std::vector<LoadedSprite> instance_data;

public:
    typedef instance_data::value_type tile_type;
    typedef instance_data::iterator iterator;
    typedef GLfloat position_t;

    Tiling(Size2<int> aCellSize, Size2<int> aGridDefinition, Size2<int> aRenderResolution);

    void resetTiling(Size2<int> aCellSize, Size2<int> aGridDefinition);

    void load(const sprites::LoadedAtlas & aAtlas);

    iterator begin();
    iterator end();

    void setBufferResolution(Size2<int> aNewResolution);

    void render() const;

    Position2<position_t> getPosition() const;
    void setPosition(Position2<position_t> aPosition);

    Rectangle<position_t> getGridRectangle() const;

    Size2<GLint> getTileSize() const;
    Size2<GLint> getGridDefinition() const;

    static constexpr GLint gTextureUnit{2};

private:
    VertexSpecification mVertexSpecification;
    Program mProgram;
    std::shared_ptr<Texture> mAtlasTexture;
    //instance_data mColors;
    instance_data mTiles;

    Size2<GLint> mTileSize;
    Size2<GLint> mGridDefinition;
    Rectangle<position_t> mGridRectangleScreen;

    static constexpr GLsizei gVerticesPerInstance{4};
};


/*
 * Implementations
 */
inline Position2<Tiling::position_t> Tiling::getPosition() const
{
    return mGridRectangleScreen.mPosition;
}

inline Rectangle<Tiling::position_t> Tiling::getGridRectangle() const
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


} // namespace graphics
} // namespace ad
