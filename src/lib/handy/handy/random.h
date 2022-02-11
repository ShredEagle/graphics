#pragma once

#include <random>

namespace ad {
// TODO Use handy namespace

template <class T_distribution = std::uniform_int_distribution<int>>
struct Randomizer
{
    typedef typename T_distribution::result_type scalar_type;
    typedef std::default_random_engine engine_type;

    Randomizer(scalar_type aMin, scalar_type aMax, scalar_type aSeed = engine_type::default_seed) :
        mMin(aMin),
        mMax(aMax),
        mEngine(aSeed),
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
