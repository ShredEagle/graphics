namespace ad {
namespace graphics{


template <class T_iterator>
std::vector<LoadedSprite> Tiling::load(T_iterator aFirst, T_iterator aLast,
                                       const Image & aRasterData)
{
    {
        Texture texture{GL_TEXTURE_RECTANGLE};
        loadSpriteSheet(texture, GL_TEXTURE1, aRasterData, aRasterData.dimension());
        mDrawContext.mTextures.push_back(std::move(texture)); 
    }

    std::vector<LoadedSprite> loadedSprites;
    std::copy(aFirst, aLast, std::back_inserter(loadedSprites));
    return loadedSprites;
}


} // namespace graphics
} // namespace ad
