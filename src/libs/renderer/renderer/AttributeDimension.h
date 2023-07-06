#pragma once


#include "GL_Loader.h"

#include <iosfwd>

#include <cassert>


namespace ad {
namespace graphics {


struct AttributeDimension
{
    /*implicit*/ constexpr AttributeDimension(GLuint aFirst, GLuint aSecond = 1) noexcept :
        mFirstDimension{aFirst},
        mSecondDimension{aSecond}
    {
        assert (mFirstDimension  <= 4);
        assert (mSecondDimension <= 4);
    }

    constexpr GLuint & operator[](std::size_t aIndex)
    { 
        assert(aIndex < 2);
        return (aIndex == 0 ? mFirstDimension : mSecondDimension);
    }

    constexpr GLuint operator[](std::size_t aIndex) const
    { 
        assert(aIndex < 2);
        return (aIndex == 0 ? mFirstDimension : mSecondDimension);
    }

    constexpr GLuint countComponents() const noexcept
    {
        return mFirstDimension * mSecondDimension;
    }

    bool operator==(AttributeDimension aRhs) const noexcept
    {
        return mFirstDimension == aRhs.mFirstDimension 
            && mSecondDimension == aRhs.mSecondDimension;
    }

    bool operator!=(AttributeDimension aRhs) const noexcept
    {
        return !(*this == aRhs);
    }

    GLuint mFirstDimension;
    GLuint mSecondDimension;

    //static constexpr AttributeDimension gScalar{1};
};


std::ostream & operator<<(std::ostream & aOut, const AttributeDimension & aAttributeDimension);


} // namespace graphics
} // namespace ad