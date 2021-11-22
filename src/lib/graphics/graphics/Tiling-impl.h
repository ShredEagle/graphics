namespace ad {
namespace graphics{


template <class T_iterator, class T_pixel>
std::vector<LoadedSprite> Tiling::load(T_iterator aFirst, T_iterator aLast,
                                       const arte::Image<T_pixel> & aRasterData)
{

    {
        Texture texture{GL_TEXTURE_RECTANGLE};
        // TODO sort out hardcoded GL_TEXTURE1
        loadSpriteSheet(texture, GL_TEXTURE1, aRasterData, aRasterData.dimensions());
        mDrawContext.mTextures.push_back(std::move(texture)); 
    }

    std::vector<LoadedSprite> loadedSprites;
    std::copy(aFirst, aLast, std::back_inserter(loadedSprites));
    return loadedSprites;
}


} // namespace graphics
} // namespace ad
