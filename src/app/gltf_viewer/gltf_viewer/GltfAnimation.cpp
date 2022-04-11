#include "GltfAnimation.h"

#include "LoadBuffer.h"
#include "Logging.h"

#include <algorithm>


namespace ad {

using namespace arte;

namespace gltfviewer {


namespace preparing {


// TODO move to a more general header.
template <class T_value>
std::vector<T_value> loadAccessorData(arte::Const_Owned<arte::gltf::Accessor> aAccessor)
{
    if (!aAccessor->bufferView)
    {
        // TODO Handle no buffer view (accessor initialized to zeros)
        ADLOG(gPrepareLogger, critical)
             ("Unsupported: accessor #{} does not have a buffer view.", aAccessor.id());
        throw std::logic_error{"Accessor without buffer view in animation."};
    }

    auto bufferView = aAccessor.get(&gltf::Accessor::bufferView);

    // TODO Handle stride (is it allowed in animation buffer views though?)
    if(bufferView->byteStride)
    {
        ADLOG(gPrepareLogger, critical)
             ("Buffer view #{} has a stride, not currently supported in animation.",
              *aAccessor->bufferView);
        throw std::logic_error{"Buffer view with stride in animation."};
    }

    // TODO Ad 2022/03/15 This can probably be optimized to work without a copy
    // by only loading the accessor bytes (in case of no-stride).
    std::vector<std::byte> completeBuffer = loadBufferData(aAccessor);

    std::vector<T_value> result;
    result.reserve(aAccessor->count);
    T_value * first = reinterpret_cast<T_value *>(completeBuffer.data() 
                                                  + bufferView->byteOffset
                                                  + aAccessor->byteOffset);
    std::copy(first, first + aAccessor->count, std::back_inserter(result));
    return result;
}


template <class T_value>
Keyframes<T_value> prepareKeyframes(arte::Const_Owned<arte::gltf::animation::Sampler> aSampler)
{
    return {
        .timestamps = loadAccessorData<GLfloat>(aSampler.get(&gltf::animation::Sampler::input)),
        .outputs = loadAccessorData<T_value>(aSampler.get(&gltf::animation::Sampler::output)),
    };
}


template <class T_value>
std::shared_ptr<Sampler> selectInterpolation(arte::Const_Owned<arte::gltf::animation::Sampler> aSampler)
{
    switch(aSampler->interpolation)
    {
    default:
        throw std::logic_error{
            "Interpolation '" + to_string(aSampler->interpolation) + "' not supported."
        };
    case gltf::animation::Sampler::Interpolation::Linear:
        return std::make_shared<SamplerLinear<T_value>>(
            prepareKeyframes<T_value>(aSampler));
    }
}


template <class T_componentType>
std::shared_ptr<Sampler> selectValue(arte::Const_Owned<gltf::animation::Sampler> aSampler)
{
    using ElementType = gltf::Accessor::ElementType;
    ElementType elementType = aSampler.get(&gltf::animation::Sampler::output)->type;
    switch(elementType)
    {
    default:
        throw std::logic_error{
            "Sampler not implement for element type '" + to_string(elementType) + "'."
        };
    case ElementType::Scalar:
        return selectInterpolation<T_componentType>(aSampler);
    case ElementType::Vec3:
        return selectInterpolation<math::Vec<3, T_componentType>>(aSampler);
    case ElementType::Vec4:
        return selectInterpolation<math::Quaternion<T_componentType>>(aSampler);
    }
}


std::shared_ptr<Sampler> selectComponent(arte::Const_Owned<gltf::animation::Sampler> aSampler)
{
    GLenum componentType = aSampler.get(&gltf::animation::Sampler::output)->componentType;
    switch(componentType)
    {
    default:
        throw std::logic_error{
            "Sampler not implement for component type '" + std::to_string(componentType) + "'."
        };
    case GL_FLOAT:
        return selectValue<GLfloat>(aSampler);
    }
}


} // namespace preparing


std::shared_ptr<Sampler> prepare(arte::Const_Owned<arte::gltf::animation::Sampler> aSampler)
{
    return preparing::selectComponent(aSampler);
}


Animation prepare(arte::Const_Owned<arte::gltf::Animation> aAnimation)
{
    Animation result;

    for (auto gltfSampler : aAnimation.iterate(&gltf::Animation::samplers))
    {
        std::shared_ptr<Sampler> sampler = prepare(gltfSampler);
        result.duration = std::max(result.duration, sampler->getDuration());
        result.samplers.push_back(std::move(sampler));
    }

    for (auto channel : aAnimation.iterate(&gltf::Animation::channels))
    {
        if(!channel->target.node)
        {
            ADLOG(gPrepareLogger, critical)
                 ("Unsupported: Channel #{} does not have a target node.", channel.id());
            throw std::logic_error{"Animation channel without a target node."};
        }

        result.nodeToChannels[*channel->target.node].push_back(
            Animation::NodeChannel{
                .path = channel->target.path,
                .sampler = result.samplers.at(channel->sampler).get(),
            }
        );
    }

    ADLOG(gPrepareLogger, debug)
         ("Loader animation #{}, total time {}s.", aAnimation.id(), result.duration);
    return result;
}


void Animation::updateScene(Time_t aTimepoint, arte::Owned<arte::gltf::Scene> aTargetScene) const
{
    switch(playMode)
    {
        case Mode::Repeat:
            aTimepoint = std::fmod(aTimepoint, duration);
            break;
        case Mode::Once:
        { /*do nothing*/ }
    }

    for (const auto & [nodeIndex, nodeChannels] : nodeToChannels)
    {
        auto node = aTargetScene.get(nodeIndex);
        arte::gltf::Node::TRS * trs = 
            std::get_if<arte::gltf::Node::TRS>(&node->transformation);

        // TODO Ad 2022/03/22: I suspect it is legal to animate a channel for
        // a node where it was not explicitly specified in the gltf.
        if(trs == nullptr)
        {
            ADLOG(gDrawLogger, critical)
                 ("Unsupported: Node #{} animates a transformation channel, but did not specify any of TRS.",
                  node.id());
            throw std::logic_error{"Animation on a node without any of TRS."};
        }

        for (auto nodeChannel : nodeChannels)
        {
            switch(nodeChannel.path)
            {
            case arte::gltf::animation::Target::Path::Translation:
                nodeChannel.sampler->interpolate(aTimepoint, trs->translation);
                break;
            case arte::gltf::animation::Target::Path::Rotation:
                nodeChannel.sampler->interpolate(aTimepoint, trs->rotation);
                break;
            case arte::gltf::animation::Target::Path::Scale:
                nodeChannel.sampler->interpolate(aTimepoint, trs->scale);
                break;
            }
        }
    }
}

} // namespace gltfviewer
} // namespace ad
