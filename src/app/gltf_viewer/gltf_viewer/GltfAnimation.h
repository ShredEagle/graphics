#pragma once


#include <arte/Gltf.h>

#include <renderer/GL_Loader.h>

#include <math/Interpolation/Interpolation.h>
#include <math/Interpolation/QuaternionInterpolation.h>


namespace ad {
namespace gltfviewer {


using Time_t = GLfloat;


template <class T_value>
struct Keyframes
{
    struct Keyframe
    {
        Time_t time;
        T_value output;
    };

    // If second keyframe is absent, it means we are clamping to an edge.
    std::pair<Keyframe, std::optional<Keyframe>> getBounds(Time_t aTimepoint) const;

    // NOTE: Kepts separate instead of the more structured vector<pair<timestamp, keyframe>>
    // so we can initialize by copy from raw buffers.
    std::vector<Time_t> timestamps;
    std::vector<T_value> outputs;
};

template <class T_value>
std::ostream & operator<<(std::ostream & aOut, const Keyframes<T_value> & aKeyframes);


struct Sampler
{
    virtual void interpolate(Time_t aTimepoint, GLfloat & aDestination) const
    { throw std::logic_error{__func__ + std::string{" not available, type error."}}; }

    virtual void interpolate(Time_t aTimepoint, math::Vec<3, GLfloat> & aDestination) const
    { throw std::logic_error{__func__ + std::string{" not available, type error."}}; }

    virtual void interpolate(Time_t aTimepoint, math::Quaternion<GLfloat> & aDestination) const
    { throw std::logic_error{__func__ + std::string{" not available, type error."}}; }

    virtual void output(std::ostream & aOut) const = 0;
};


template <class T_value>
struct SamplerLinear : public Sampler
{
    SamplerLinear(Keyframes<T_value> aKeyframes) :
        keyframes{std::move(aKeyframes)}
    {}

    void interpolate(Time_t aTimepoint, T_value & aDestination) const override;

    void output(std::ostream & aOut) const override;

    Keyframes<T_value> keyframes;
};


std::shared_ptr<Sampler> prepare(arte::Const_Owned<arte::gltf::Sampler> aSampler);


struct Animation
{
    struct NodeChannel
    {
        arte::gltf::Target::Path path;
        Sampler * sampler;
    };

    std::vector<std::shared_ptr<Sampler>> samplers;
    std::map<arte::gltf::Index<arte::gltf::Node>, NodeChannel> nodeToChannel;
};


Animation prepare(arte::Const_Owned<arte::gltf::Animation> aAnimation);


//
// Implementations
//
template <class T_value>
std::ostream & operator<<(std::ostream & aOut, const Keyframes<T_value> & aKeyframes)
{
    auto & timestamps = aKeyframes.timestamps;
    auto & outputs = aKeyframes.outputs;

    std::size_t id = 0;
    if(id != timestamps.size())
    {
        aOut << timestamps[id] << ": " << outputs[id];
    }
    for (++id; id != timestamps.size(); ++id)
    {
        aOut << ", " << timestamps[id] << ": " << outputs[id];
    }

    return aOut;
}


template <class T_value>
std::pair<typename Keyframes<T_value>::Keyframe, std::optional<typename Keyframes<T_value>::Keyframe> >
Keyframes<T_value>::getBounds(Time_t aTimepoint) const
{
    std::size_t id = 0;

    if (timestamps[id] >= aTimepoint)
    {
        return { 
            {timestamps[id], outputs[id]},
            std::nullopt 
        };
    }

    for(++id; id < timestamps.size(); ++id)
    {
        if(timestamps[id] >= aTimepoint)
        {
            return { 
                Keyframe{timestamps[id-1], outputs[id-1]},
                Keyframe{timestamps[id], outputs[id]},
            };
        }
    }

    return { 
        {timestamps.back(), outputs.back()},
        std::nullopt 
    };
}


template <class T_value>
void SamplerLinear<T_value>::interpolate(Time_t aTimepoint, T_value & aDestination) const
{
    auto [firstBound, optionalBound] = keyframes.getBounds(aTimepoint);

    if(optionalBound)
    {
        GLfloat interpolationParam = 
            (aTimepoint - firstBound.time) / (optionalBound->time - firstBound.time);

        // TODO 1 slerp on quaternions
        aDestination = math::lerp(
            optionalBound->output,
            firstBound.output,
            math::Clamped{interpolationParam});
    }
    else
    {
        aDestination = firstBound.output;
    }
}


template <class T_value>
void SamplerLinear<T_value>::output(std::ostream & aOut) const
{
    aOut << "<SamplerLinear> {" << keyframes << "}";
}


} // namespace gltfviewer
} // namespace ad
