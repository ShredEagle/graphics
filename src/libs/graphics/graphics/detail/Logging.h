#pragma once

#include <spdlog/spdlog.h>


#define ADLOG(logger, severity) spdlog::get(logger)->severity


namespace ad {
namespace graphics {


constexpr const char * gMainLogger = "logging";
constexpr const char * gOpenglLogger = "opengl";


namespace detail {


/// Will be called on AppInterface construction, must be called explicitly if it is not instanciated.
void initializeLogging();


} // namespace detail
} // namespace graphics
} // namespace ad
