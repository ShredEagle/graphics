#pragma once


#include <arte/gltf/Gltf.h>

#include <renderer/GL_Loader.h>

#include <math/Interpolation/Interpolation.h>
#include <math/Interpolation/QuaternionInterpolation.h>

#include <memory>


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

    virtual Time_t getDuration() const = 0;
};


template <class T_value>
struct SamplerLinear : public Sampler
{
    SamplerLinear(Keyframes<T_value> aKeyframes) :
        keyframes{std::move(aKeyframes)}
    {}

    void interpolate(Time_t aTimepoint, T_value & aDestination) const override;

    void output(std::ostream & aOut) const override;

    Time_t getDuration() const override
    { return keyframes.timestamps.back(); }

    Keyframes<T_value> keyframes;
};


std::shared_ptr<Sampler> prepare(arte::Const_Owned<arte::gltf::animation::Sampler> aSampler);


struct Animation
{
    struct NodeChannel
    {
        arte::gltf::animation::Target::Path path;
        Sampler * sampler;
    };

    enum class Mode
    {
        Once,
        Repeat,
    };

    void updateScene(Time_t aTimepoint, arte::Owned<arte::gltf::Scene> aTargetScene) const;

    std::vector<std::shared_ptr<Sampler>> samplers;
    // > Within one animation, each target (a combination of a node and a path)
    // > MUST NOT be used more than once.
    std::map<arte::gltf::Index<arte::gltf::Node>, 
             std::vector<NodeChannel>> nodeToChannels;

    Mode playMode{Mode::Repeat};
    Time_t duration{0};
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

        if constexpr (std::is_same_v<math::Quaternion<GLfloat>, T_value>)
        {
            aDestination = math::slerp(
                firstBound.output,
                optionalBound->output,
                math::Clamped{interpolationParam});
        }
        else
        {
            aDestination = math::lerp(
                firstBound.output,
                optionalBound->output,
                math::Clamped{interpolationParam});
        }
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
