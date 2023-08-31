#pragma once 


#include <test_commons/PathProvider.h>


namespace ad
{
    

inline filesystem::path ensureTemporaryImageFolder(filesystem::path aSubfolder)
{
    filesystem::path tmp =
#if defined(__APPLE__)
        "/tmp"; // not at all what is returned on macos by temp_directory_path()
#else
        filesystem::temp_directory_path();
#endif

    filesystem::path result =  tmp / aSubfolder;
    create_directories(result);
    return result;

}


} // namespace ad