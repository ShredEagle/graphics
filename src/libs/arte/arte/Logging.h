#pragma once

#include <spdlog/spdlog.h>


#define ADLOG(logger, severity) spdlog::get(logger)->severity


namespace ad {
namespace arte {


constexpr const char * gMainLogger = "arte";


void initializeLogging();


} // namespace arte
} // namespace ad
