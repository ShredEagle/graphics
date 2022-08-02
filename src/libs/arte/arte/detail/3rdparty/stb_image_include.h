#include <algorithm>
#include <cstring>


namespace ad_stbi
{
    inline unsigned char * ad_realloc(unsigned char * p, std::size_t oldsz, std::size_t newsz)
    {
        unsigned char * dst = new unsigned char[newsz];
        std::memcpy(dst, p, std::min(oldsz, newsz));
        delete [] p;
        return dst;
    }
} // namespace ad_stbi


#define STBI_MALLOC(sz)         new unsigned char[sz]
#define STBI_FREE(p)            delete [] (unsigned char *)p
#define STBI_REALLOC_SIZED(p,oldsz,newsz) ad_stbi::ad_realloc((unsigned char *)p, oldsz, newsz)

#include "stb_image.h"
