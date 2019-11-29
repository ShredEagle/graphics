#include "FileWatcher.h"

namespace ad {

std::map<std::filesystem::path, std::filesystem::file_time_type>
initMap(std::initializer_list<std::filesystem::path> aWatchlist)
{
    std::map<std::filesystem::path, std::filesystem::file_time_type> result;
    for (const std::filesystem::path & path : aWatchlist)
    {
        result.emplace(path, std::filesystem::last_write_time(path));
    }
    return result;
}

FileWatcher::FileWatcher(std::initializer_list<path> aWatchlist) :
    mWatch(initMap(std::move(aWatchlist)))
{}

bool FileWatcher::check()
{
    bool changed{false};
    for (auto & node : mWatch)
    {
        auto lastRead = std::filesystem::last_write_time(node.first);
        if (node.second != lastRead)
        {
            changed = true;
            node.second = lastRead;
        }
    }
    return changed;
}

} // namespace ad
