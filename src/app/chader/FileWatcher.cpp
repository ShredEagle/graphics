#include "FileWatcher.h"

using path=boost::filesystem::path;

namespace ad {

std::map<path, std::time_t>
initMap(std::initializer_list<path> aWatchlist)
{
    std::map<path, std::time_t> result;
    for (const path & path : aWatchlist)
    {
        result.emplace(path, boost::filesystem::last_write_time(path));
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
        auto lastRead = boost::filesystem::last_write_time(node.first);
        if (node.second != lastRead)
        {
            changed = true;
            node.second = lastRead;
        }
    }
    return changed;
}

} // namespace ad
