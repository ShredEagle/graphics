#pragma once

#include "build_config.h"

#include <platform/Filesystem.h>

#include <string>


namespace ad {


inline filesystem::path pathFor(const filesystem::path &aAsset)
{
    auto result = gAssetFolderPath / aAsset;
    if (!exists(result))
    {
        throw std::runtime_error{result.string() + " does not exist."};
    }
    return result;
}


} // namespace ad


