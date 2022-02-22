#include "Logging.h"

#include "spdlog/sinks/stdout_color_sinks.h"


namespace ad {
namespace graphics {
namespace detail {


/// This singleton has no state, its purpose is to initialize graphics
/// logging on construction.
/// Being a singleton, it ensures the initialization occurs at most once.
struct LoggingInitializationSingleton
{
    LoggingInitializationSingleton()
    {
        // Intended for message issued by opengl
        spdlog::stdout_color_mt("opengl"); 
        // Intended for the actual library messages
        spdlog::stdout_color_mt("graphics"); 
    }
};


void initializeLogging()
{
    static LoggingInitializationSingleton initializeOnce;
};


} // namespace detail
} // namespace graphics
} // namespace ad
