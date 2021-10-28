#pragma once


#include <platform/Filesystem.h>

#include <stdexcept>
#include <string>


namespace ad {
namespace resource {

/// \brief Singleton class that can be initialized, then is implicitly accessed by pathFor()
class ResourceManager
{
public:
    ResourceManager(const filesystem::path aRoot) :
        mRoot{std::move(aRoot)}
    {}

    // Not copyable
    ResourceManager(const ResourceManager &) = delete;
    ResourceManager & operator=(const ResourceManager &) = delete;
    // TODO make movable

    filesystem::path pathFor(const filesystem::path &aAsset) const;

private:
    filesystem::path mRoot;
};


inline filesystem::path ResourceManager::pathFor(const filesystem::path &aAsset) const
{
    auto result = mRoot / aAsset;
    if (!exists(result))
    {
        throw std::runtime_error{result.string() + " does not exist."};
    }
    return result;
}


} // namespace resource
} // namespace ad
