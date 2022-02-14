#include "Path.h"


#include <boost/dll/runtime_symbol_info.hpp>

namespace ad {
namespace platform {


filesystem::path getExecutableFilePath()
{
    return boost::dll::program_location();
}


} // namespace platform
} // namespace ad
