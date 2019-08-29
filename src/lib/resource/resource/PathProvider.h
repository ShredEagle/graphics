#pragma once

#include "build_config.h"

#include <string>

namespace ad {

std::string pathFor(const std::string &aAsset)
{
    return gAssetFolderPath + ("/" + aAsset);
}

} // namespace ad


