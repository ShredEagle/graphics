#include "AttributeDimension.h"


#include <ostream>


namespace ad {
namespace graphics {


std::ostream & operator<<(std::ostream & aOut, const AttributeDimension & aAttributeDimension)
{
    if (aAttributeDimension.mSecondDimension == 1)
    {
        return aOut << aAttributeDimension.mFirstDimension;
    }
    else
    {
        return aOut << "(" << aAttributeDimension[0] << ", " << aAttributeDimension[1] << ")";
    }
}


} // namespace graphics
} // namespace ad