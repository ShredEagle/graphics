#include "Image.h"

#include "details/ImageFormats/Ppm.h"

#include <algorithm>
#include <string>
#include <istream>

namespace ad {


template <class T_pixelFormat>
Image<T_pixelFormat>::Image(math::Size<2, int> aDimensions, std::unique_ptr<char[]> aRaster) :
    mDimensions{aDimensions},
    mRaster{std::move(aRaster)}
{}


template <class T_pixelFormat>
Image<T_pixelFormat>::Image(math::Size<2, int> aDimensions, T_pixelFormat aBackgroundValue) :
    mDimensions{aDimensions},
    mRaster{new char[mDimensions.area()*pixel_size_v]}
{
    std::fill(begin(), end(), aBackgroundValue);
}


template <class T_pixelFormat>
Image<T_pixelFormat>::Image(const Image & aRhs) :
    mDimensions(aRhs.mDimensions),
    mRaster{new char[mDimensions.area()*pixel_size_v]}
{
    std::copy(aRhs.begin(), aRhs.end(), begin());
}


template <class T_pixelFormat>
Image<T_pixelFormat> & Image<T_pixelFormat>::operator=(const Image & aRhs)
{
    *this = Image{aRhs};
    return *this;
}


template <class T_pixelFormat>
void Image<T_pixelFormat>::writePpm(std::ostream & aOut) const
{
    detail::Ppm::Write(aOut, *this);
}


template <class T_pixelFormat>
Image<T_pixelFormat> Image<T_pixelFormat>::ReadPpm(std::istream & aIn)
{
    return detail::Ppm::Read(aIn);
}


//template <class T_pixelFormat>
//Image<T_pixelFormat> Image<T_pixelFormat>::LoadFile(const path & aImageFile)
//{
//    path extension = aImageFile.extension();
//    if(extension == ".ppm")
//    {
//        return detail::Ppm::Read(std::ifstream(aImageFile, std::ios_base::in | std::ios_base::binary));
//    }
//}


//
// Explicit instantiations
//
template class Image<>;

} // namespace ad

