#pragma once


#include "gl_helpers.h"
#include "GL_Loader.h"

#include <handy/Guard.h>


namespace ad::graphics {



struct [[nodiscard]] Query : public ResourceGuard<GLuint>
{
    Query() :
        ResourceGuard<GLuint>(reserve(glGenQueries),
                              [](GLuint queryId){glDeleteQueries(1, &queryId);})
    {}
};


} // namespace ad::graphics