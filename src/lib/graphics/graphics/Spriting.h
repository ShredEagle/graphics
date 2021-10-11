#pragma once

#include "commons.h"

#include "Sprite.h"

#include <renderer/Drawing.h>
#include <vector>


namespace ad {


struct Instance
{
    Instance(Position2<GLint> aPosition, LoadedSprite aTextureArea):
        mPosition(aPosition),
        mTextureArea(aTextureArea)
    {}

    Position2<GLint> mPosition;
    LoadedSprite mTextureArea;
};

//const std::vector<const AttributeDescription> gInstanceDescription = {
//    {2, 2, offsetof(Instance, mPosition),    MappedGL<GLint>::enumerator},
//    {3, 4, offsetof(Instance, mTextureArea), MappedGL<GLint>::enumerator, ShaderAccess::Integer},
//};


/// \brief Draws a list of sprites (loaded from a single spritesheet with ::load()) at given positions.
///
/// The instance data is a vector of association between a position and a sprite (in the spritesheet).
class Spriting
{
public:
    typedef std::vector<Instance>  instance_data;
    typedef instance_data::iterator iterator;

    Spriting(Size2<int> aRenderResolution);

    /// \brief Takes a pair of iterator to SpriteArea instances, and the corresponding raster data
    template <class T_iterator>
    std::vector<LoadedSprite> load(T_iterator aFirst, T_iterator aLast,
                                   const Image & aRasterData);


    instance_data & instanceData();
    iterator begin();
    iterator end();

    void render() const;

    void setBufferResolution(Size2<int> aNewResolution);

private:
    DrawContext mDrawContext;
    instance_data mSprites;
};


/*
 * Implementation
 */

inline Spriting::instance_data & Spriting::instanceData()
{
    return mSprites;
}

inline Spriting::iterator Spriting::begin()
{
    return mSprites.begin();
}

inline Spriting::iterator Spriting::end()
{
    return mSprites.end();
}

template <class T_iterator>
std::vector<LoadedSprite> Spriting::load(T_iterator aFirst, T_iterator aLast,
                                         const Image & aRasterData)
{
    {
        Texture texture{GL_TEXTURE_RECTANGLE};
        loadSpriteSheet(texture, GL_TEXTURE0, aRasterData, aRasterData.dimension());
        mDrawContext.mTextures.push_back(std::move(texture));
    }

    std::vector<LoadedSprite> loadedSprites;
    std::copy(aFirst, aLast, std::back_inserter(loadedSprites));
    return loadedSprites;
}

} // namespace ad
