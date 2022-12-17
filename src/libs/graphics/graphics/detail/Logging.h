#pragma once

#include <spdlog/spdlog.h>


#define ADLOG(logger, severity) spdlog::get(logger)->severity


namespace ad {
namespace graphics {


constexpr const char * gMainLogger = "graphics";
constexpr const char * gOpenglLogger = "graphics::opengl";


namespace detail {


/// Will be called on AppInterface construction, must be called explicitly if it is not instanciated.
void initializeLogging();


} // namespace detail
} // namespace graphics
} // namespace ad
