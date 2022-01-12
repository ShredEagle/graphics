#pragma once

#include "commons.h"

#include "Sprite.h"

#include <math/Homogeneous.h>

#include <renderer/Drawing.h>


namespace ad {
namespace graphics{


class AppInterface;


class Tiling
{
public:
    using Instance = LoadedSprite;
    using Position_t = GLfloat;

    Tiling(Size2<int> aCellSize, Size2<int> aGridDefinition);

    /// \brief Reset the geometry tile grid to be rendered.
    void resetTiling(Size2<int> aCellSize, Size2<int> aGridDefinition);

    /// \brief Associate the atlas to be used when rendering.
    void load(const sprites::LoadedAtlas & aAtlas);

    void updateInstances(gsl::span<const Instance> aInstances);

    /// \brief Render all instances, using the associated atlas.
    void render() const;

    void setCameraTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation);
    void setProjectionTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation);

    Position2<Position_t> getPosition() const;
    void setPosition(Position2<Position_t> aPosition);

    Rectangle<Position_t> getGridRectangle() const;

    std::size_t getTileCount() const;
    Size2<GLint> getTileSize() const;
    Size2<GLint> getGridDefinition() const;

    static constexpr GLint gTextureUnit{2};
    /// \brief Notably usefull to initialize collection of instances.
    static inline const Instance gEmptyInstance{ {0, 0}, {0, 0} };

private:
    VertexSpecification mVertexSpecification;
    Program mProgram;
    std::shared_ptr<Texture> mAtlasTexture;

    Size2<GLint> mTileSize;
    Size2<GLint> mGridDefinition;
    Rectangle<Position_t> mGridRectangleScreen;

    GLsizei mInstanceCount{0};

    static constexpr GLsizei gVerticesPerInstance{4};
};


/*
 * Implementations
 */
inline Position2<Tiling::Position_t> Tiling::getPosition() const
{
    return mGridRectangleScreen.mPosition;
}

inline Rectangle<Tiling::Position_t> Tiling::getGridRectangle() const
{
    return mGridRectangleScreen;
}

inline::std::size_t Tiling::getTileCount() const
{
    return mGridDefinition.area();
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
