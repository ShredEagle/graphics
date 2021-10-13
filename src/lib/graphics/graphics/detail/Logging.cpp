#include "Logging.h"

#include "spdlog/sinks/stdout_color_sinks.h"


namespace ad {
namespace graphics {
namespace detail {


void initializeLogging()
{
    // Intended for message issued by opengl
    spdlog::stdout_color_mt("opengl"); 
    // Intended for the actual library messages
    spdlog::stdout_color_mt("graphics"); 
};


} // namespace detail
} // namespace graphics
} // namespace ad
