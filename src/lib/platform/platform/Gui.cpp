#include "Gui.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <iostream>
#endif

namespace ad {
namespace platform {


#if defined(_WIN32)

void showErrorBox(const std::string & aMessage, const std::string & aTitle)
{
    MessageBox(nullptr,
               aMessage.c_str(),
               aTitle.empty() ? nullptr : aTitle.c_str(),
               MB_OK | MB_ICONERROR);
}

#else

// TODO Ad 2022/02/22: Implement for other platforms.
void showErrorBox(const std::string & aMessage, const std::string & aTitle)
{
    std::cerr 
        << (aTitle.empty() ? "Error: " : aTitle + ": ")
        << aMessage 
        << "\n"
        ;
}

#endif


} // namespace platform
} // namespace ad
