#pragma once

#include "ResourceLocator.h"

#include <platform/Filesystem.h>

#include <handy/StringId.h>

#include <unordered_map>


namespace ad {
namespace resource {

// TODO Make this class safer to use (currently it is very easy to get dangling references).
// TODO Ad 2022/02/10 There should be a separate case where the loader has states 
// (i.e. an instance is stored withing the ResourceManager data members).
/// \attention The resources are currently hosted directly by the ResourceManager, not 
/// inside any sharing mechanism.
/// It implies that the resource handles are only valid while the ResourceManager is alive,
/// and while it contains them. This is currently left as the user responsibility.
template <class T_resource, auto F_loader>
class ResourceManager
{
public:
    ResourceManager() = default;

    // Not copyable because it returns pointers/references to resources it owns
    ResourceManager(const ResourceLocator &) = delete;
    ResourceManager & operator=(const ResourceLocator &) = delete;
    ResourceManager(ResourceLocator &&) = delete;
    ResourceManager & operator=(ResourceLocator &&) = delete;

    template <class ...VT_loaderParams>
    const T_resource & load(filesystem::path aAssetPath, const ResourceLocator & aLocator, VT_loaderParams &&...);
    /// \attention Invalidates all references to the removed resource.
    void remove(filesystem::path aAssetPath);

private:
    std::unordered_map<handy::StringId, T_resource> mResourceTable;
};


//
// Implementations
//
template <class T_resource, auto F_loader>
template <class ...VT_loaderParams>
const T_resource & ResourceManager<T_resource, F_loader>::load(filesystem::path aAssetPath, const ResourceLocator & aLocator, VT_loaderParams &&... aLoaderParams)
{
    handy::StringId resourceId{aAssetPath.string()};

    if (auto foundIt = mResourceTable.find(resourceId);
        foundIt == mResourceTable.end())
    {
        // The resource is not present in the table, it has to be loaded
        return mResourceTable.emplace(resourceId, F_loader(aLocator.pathFor(aAssetPath), std::forward<VT_loaderParams>(aLoaderParams)...)).first->second;
    }
    else
    {
        return foundIt->second;
    }
}


template <class T_resource, auto F_loader>
void ResourceManager<T_resource, F_loader>::remove(filesystem::path aAssetPath)
{
    mResourceTable.erase(handy::StringId{aAssetPath.string()});
}


} // namespace resource
} // namespace ad
