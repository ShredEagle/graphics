#include "Locale.h"

#include <boost/locale.hpp>


namespace ad {
namespace platform {


std::string getSystemLanguage()
{
    boost::locale::generator gen;
    return std::use_facet<boost::locale::info>(gen("")).language();
}


} // namespace platform
} // namespace ad
