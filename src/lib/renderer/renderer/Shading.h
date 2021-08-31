#pragma once

#include <handy/Guard.h>
#include <handy/tuple_utils.h>

#include "GL_Loader.h"

#include <functional>
#include <vector>

namespace ad
{

struct [[nodiscard]] Shader : public ResourceGuard<GLuint>
{
    Shader(GLenum aStage) :
        ResourceGuard<GLuint>{glCreateShader(aStage), glDeleteShader},
        mStage(aStage)
    {}

    Shader(GLenum aStage, const char * aSource);

    GLenum mStage;
};


struct [[nodiscard]] Program : public ResourceGuard<GLuint>
{
    Program() :
        ResourceGuard<GLuint>{glCreateProgram(), glDeleteProgram}
    {}
};

void handleGlslError(GLuint objectId,
                     GLenum aStatusEnumerator,
                     std::function<void(GLuint, GLenum, GLint*)> statusGetter,
                     std::function<void(GLuint, GLsizei, GLsizei*, GLchar*)> infoLogGetter);

class ShaderCompilationError : public std::runtime_error
{
public:

    ShaderCompilationError(const std::string & aWhat, const std::string & aErrorLog) :
        std::runtime_error::runtime_error(aWhat + " Log: " + aErrorLog),
        mErrorLog(aErrorLog)
    {}

    const std::string & getErrorLog() const
    {
        return mErrorLog;
    }

private:
    std::string mErrorLog;
};

void compileShader(const Shader & aShader, const char * aSource);

Program makeLinkedProgram(std::initializer_list<std::pair<const GLenum/*stage*/,
                                                          const char * /*source*/>> aShaders);


} // namespace ad
