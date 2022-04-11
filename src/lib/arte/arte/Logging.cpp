#include "Logging.h"

#include "spdlog/sinks/stdout_color_sinks.h"


namespace ad {
namespace arte {


namespace {

    /// This singleton has no state, its purpose is to initialize graphics
    /// logging on construction.
    /// Being a singleton, it ensures the initialization occurs at most once.
    struct LoggingInitializationSingleton
    {
        LoggingInitializationSingleton()
        {
            spdlog::stdout_color_mt(gMainLogger);
        }
    };

} // namespace anonymous


void initializeLogging()
{
    static LoggingInitializationSingleton initializeOnce;
};


} // namespace arte
} // namespace ad
