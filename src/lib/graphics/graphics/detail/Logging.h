#pragma once

#include <spdlog/spdlog.h>


#define LOG(logger, severity) spdlog::get(#logger)->severity


namespace ad {
namespace graphics {
namespace detail {


/// Will be called on AppInterface construction, must be called explicitly if it is not instanciated.
void initializeLogging();


} // namespace detail
} // namespace graphics
} // namespace ad
