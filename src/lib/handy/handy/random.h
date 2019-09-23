#pragma once

#include <random>

namespace ad {

template <class T_distribution = std::uniform_int_distribution<int>>
struct Randomizer
{
    typedef typename T_distribution::result_type scalar_type;

    Randomizer(scalar_type aMin, scalar_type aMax) :
        mMin(aMin),
        mMax(aMax),
        mDistribution(aMin, aMax)
    {}

    scalar_type operator()()
    {
        return mDistribution(mEngine);
    }

    //template <class T>
    //T norm()
    //{
    //    return mDistribution(mEngine)/static_cast<T>(mMax);
    //}

    scalar_type mMin;
    scalar_type mMax;
    std::default_random_engine mEngine{};
    T_distribution mDistribution;
};

} // namespace ad
