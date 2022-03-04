#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>


#define ADLOG(logger, severity) spdlog::get(logger)->severity


namespace ad {
namespace gltfviewer {


constexpr const char * gMainLogger = "gltfviewer";


void initializeLogging();


} // namespace gltfviewer;
} // namespace ad
