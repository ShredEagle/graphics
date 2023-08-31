#pragma once

#include "Image.h"

#include <math/Vector.h>

#include <functional>
#include <numbers>


namespace ad::arte {


using FilterFunc_t = float(*)(float);


template <FilterFunc_t F_filter>
struct StaticFilter
{
    float operator()(float aValue)
    {
        return F_filter(aValue);
    }

    float mRadius;
};


struct Filter
{
    float operator()(float aValue)
    {
        return mFilterFunc(aValue);
    }

    std::function<float(float)> mFilterFunc;
    float mRadius;
};



// TODO this could be generalized to any type of sequence (in any dimension), thus moving to math
template <class T_pixelFormat, class T_filter>
Image<T_pixelFormat> resampleSeparable2D(const Image<T_pixelFormat> & aInput,
                                         math::Size<2, int> aOutputResolution,
                                         T_filter aFilter)
{
    const float r = aFilter.mRadius;

    const auto inputDimensions = dimensions(aInput);

    math::Vec<2, float> delta = 
        math::Vec<2, float>{dimensions(aInput)}.cwDiv(static_cast<math::Vec<2, float>>(aOutputResolution));

    auto intermediary = arte::Image<T_pixelFormat>::makeUninitialized(
                            {aOutputResolution.width(), (int)aInput.height()});

    // With the convention that the image domain is (-0.5, Nx - 0.5) x (-0.5, Ny - 0.5)
    float x0 = -0.5f + delta.x()/2;
    float y0 = -0.5f + delta.y()/2;

    // Resample all the rows of the source
    for (std::size_t i = 0; i != aInput.height(); ++i)
    {
        // For each pixel in the (intermediary) output row
        for (std::size_t j = 0; j != aOutputResolution.width(); ++j)
        {
            intermediary[j][i] = T_pixelFormat{}; // assign zero
            // x coordinate of the output pixel, expressed in the input grid
            float x = x0 + j * delta.x();
            // For each pixel **center** that falls within the radius of the filter
            // (the filter being centered on x, i.e. the output pixel)
            for (std::size_t k = (std::size_t)std::max<float>(std::ceil(x - r), 0);
                 k <= std::min<std::size_t>((std::size_t)std::floor(x + r), aInput.width() - 1);
                 ++k)
            {
                intermediary[j][i] += aInput[k][i] * aFilter(x - k);
            }
        }
    }

    auto output = arte::Image<T_pixelFormat>::makeUninitialized(aOutputResolution);

    // Resample all the columns of the intermediary
    for (std::size_t j = 0; j != aOutputResolution.width(); ++j)
    {
        for (std::size_t i = 0; i != aOutputResolution.height(); ++i)
        {
            output[j][i] = T_pixelFormat{}; // assign zero
            float y = y0 + i * delta.y();
            for (std::size_t k = (std::size_t)std::max<float>(std::ceil(y - r), 0);
                 k <= std::min<std::size_t>((std::size_t)std::floor(y + r), intermediary.height() - 1);
                 ++k)
            {
                output[j][i] += intermediary[j][k] * aFilter(y - k);
            }
        }
    }

    return output;
}


template <class T_value = float>
T_value gaussian(T_value x, T_value sigma, T_value scale)
{
    x = x / scale;
    T_value evaluation = T_value{1} / (sigma * std::sqrt(2 * std::numbers::pi_v<T_value>))
        * std::exp(-std::pow(x, T_value{2}) / (2 * std::pow(sigma, T_value{2})));
    return evaluation / scale;
}


template <class T_value = float>
T_value catmullRom(T_value x, T_value scale = 1)
{
    x = x / scale;

    T_value xp = std::abs(x);
    T_value evaluation;
    if ( xp <= 1)
    {
        evaluation = -3 * std::pow(1 - xp, T_value{3}) + 4 * std::pow(1 - xp, T_value{2}) + 1 - xp;
    }
    else if (xp <= 2)
    {
        evaluation = std::pow(2 - xp, T_value{3}) - std::pow(2 - xp, T_value{2});
    }
    else
    {
        evaluation = 0;
    }
    
    evaluation /= 2;
    return evaluation / scale;
}


// TODO limit to sdr images
template <class T_pixelFormat>
Image<T_pixelFormat> resampleImage(const Image<T_pixelFormat> & aInput,
                                   math::Size<2, int> aOutputResolution)
{
    return tonemap(resampleSeparable2D(to_hdr(aInput),
                                       aOutputResolution,
                                       Filter{.mFilterFunc = [](float x){return catmullRom(x);}, .mRadius = 2.}));
}


} // namespace ad::arte