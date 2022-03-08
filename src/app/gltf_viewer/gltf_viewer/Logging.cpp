#include "Logging.h"

#include "spdlog/sinks/stdout_color_sinks.h"


namespace ad {
namespace gltfviewer {


namespace {

    /// This singleton has no state, its purpose is to initialize graphics
    /// logging on construction.
    /// Being a singleton, it ensures the initialization occurs at most once.
    struct LoggingInitializationSingleton
    {
        LoggingInitializationSingleton()
        {
            spdlog::stdout_color_mt(gPrepareLogger);
            spdlog::stdout_color_mt(gDrawLogger);
        }
    };

} // namespace anonymous


void initializeLogging()
{
    static LoggingInitializationSingleton initializeOnce;
};


} // namespace gltfviewer
} // namespace ad
