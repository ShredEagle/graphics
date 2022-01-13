#pragma once

#include "commons.h"

#include "Sprite.h"

#include <math/Homogeneous.h>

#include <renderer/Drawing.h>


namespace ad {
namespace graphics{


class AppInterface;


class TileSet
{
    friend class Tiling;

public:
    using Instance = LoadedSprite;
    using Position_t = GLfloat;

    TileSet(Size2<int> aCellSize, Size2<int> aGridDefinition);

    void updateInstances(gsl::span<const Instance> aInstances);

    /// \brief Reset the geometry tile grid to be rendered.
    void resetTiling(Size2<int> aCellSize, Size2<int> aGridDefinition);

    Position2<TileSet::Position_t> getPosition() const;
    void setPosition(Position2<TileSet::Position_t> aPosition);

    Rectangle<Position_t> getGridRectangle() const;

    std::size_t getTileCount() const;
    Size2<GLint> getTileSize() const;
    Size2<GLint> getGridDefinition() const;

    /// \brief Notably usefull to initialize collection of instances.
    static inline const Instance gEmptyInstance{ {0, 0}, {0, 0} };

private:
    VertexSpecification mVertexSpecification;

    Size2<GLint> mTileSize;
    Size2<GLint> mGridDefinition;
    Rectangle<Position_t> mGridRectangleScreen;

    static constexpr GLsizei gVerticesPerInstance{4};
};


class Tiling
{
public:
    Tiling();

    /// \brief Render all instances, using the associated atlas.
    void render(const sprites::LoadedAtlas & aAtlas, const TileSet & aTileSet) const;

    void setCameraTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation);
    void setProjectionTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation);

    static constexpr GLint gTextureUnit{2};

private:
    // TODO Ad 2022/01/13: This has a very strong smell, but the position of the grid is set as a uniform
    // in the program (and that cannot render() member functions are intended to be const).
    mutable Program mProgram;
};


/*
 * Implementations
 */
inline Position2<TileSet::Position_t> TileSet::getPosition() const
{
    return mGridRectangleScreen.mPosition;
}


inline void TileSet::setPosition(Position2<TileSet::Position_t> aPosition)
{
    mGridRectangleScreen.mPosition = aPosition;
}


inline Rectangle<TileSet::Position_t> TileSet::getGridRectangle() const
{
    return mGridRectangleScreen;
}

inline::std::size_t TileSet::getTileCount() const
{
    return mGridDefinition.area();
}

inline Size2<GLint> TileSet::getTileSize() const
{
    return mTileSize;
}

inline Size2<GLint> TileSet::getGridDefinition() const
{
    return mGridDefinition;
}


} // namespace graphics
} // namespace ad
