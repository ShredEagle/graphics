#pragma once

#include <platform/Filesystem.h>

#include <handy/StringId.h>

#include <unordered_map>


namespace ad {
namespace resource {


// TODO Make this class safer to use (currently it is very easy to get dangling references).
/// \attention The resources are currently hosted directly by the ResourceManager, not 
/// inside any sharing mecanism.
/// It implies that the resource handles are only valid while the ResourceManager is alive,
/// and while it contains them. This is currently left as the user responsibility.
template <class T_resource, T_resource(* F_loader)(const filesystem::path &)>
class ResourceManager
{
public:
    ResourceManager() = default;

    // Not copyable because it returns pointers/references to resources it owns
    ResourceManager(const ResourceLocator &) = delete;
    ResourceManager & operator=(const ResourceLocator &) = delete;
    ResourceManager(ResourceLocator &&) = delete;
    ResourceManager & operator=(ResourceLocator &&) = delete;

    const T_resource & load(filesystem::path aAssetPath, const ResourceLocator & aLocator);
    /// \attention Invalidates all references to the removed resource.
    void remove(filesystem::path aAssetPath);

private:
    std::unordered_map<handy::StringId, T_resource> mResourceTable;
};


//
// Implementations
//
template <class T_resource, T_resource(* F_loader)(const filesystem::path &)>
const T_resource & ResourceManager<T_resource, F_loader>::load(filesystem::path aAssetPath, const ResourceLocator & aLocator)
{
    handy::StringId resourceId{aAssetPath.string()};

    if (auto foundIt = mResourceTable.find(resourceId);
        foundIt == mResourceTable.end())
    {
        // The resource is not present in the table, it has to be loaded
        return mResourceTable.emplace(resourceId, F_loader(aLocator.pathFor(aAssetPath))).first->second;
    }
    else
    {
        return foundIt->second;
    }
}


template <class T_resource, T_resource(* F_loader)(const filesystem::path &)>
void ResourceManager<T_resource, F_loader>::remove(filesystem::path aAssetPath)
{
    mResourceTable.erase(StringId{aAssetPath});
}


} // namespace resource
} // namespace ad