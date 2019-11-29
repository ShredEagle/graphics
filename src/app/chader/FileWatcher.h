#pragma once

#include <filesystem>
#include <map>

namespace ad {

class FileWatcher
{
    using path=std::filesystem::path;

public:
    FileWatcher(std::initializer_list<path> aWatchlist);

    bool check();

private:
    std::map<path, std::filesystem::file_time_type> mWatch;
    //std::thread mWorker;
    //std::atomic<bool> mChanged{false};
};



} // namespace ad
