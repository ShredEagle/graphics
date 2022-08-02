#pragma once

#include <boost/filesystem.hpp>
#include <map>

namespace ad {

class FileWatcher
{
    using path=boost::filesystem::path;

public:
    FileWatcher(std::initializer_list<path> aWatchlist);

    bool check();

private:
    std::map<path, std::time_t> mWatch;
    //std::thread mWorker;
    //std::atomic<bool> mChanged{false};
};



} // namespace ad
