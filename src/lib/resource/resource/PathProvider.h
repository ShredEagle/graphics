/// \deprecated Do not include this file, it only works in build tree.

#pragma once

#include "build_config.h"

#include <platform/Filesystem.h>

#include <stdexcept>
#include <string>


namespace ad {
namespace resource {


inline filesystem::path pathFor(const filesystem::path &aAsset)
{
    auto result = gAssetFolderPath / aAsset;
    if (!exists(result))
    {
        throw std::runtime_error{result.string() + " does not exist."};
    }
    return result;
}


} // namespace resource
} // namespace ad


