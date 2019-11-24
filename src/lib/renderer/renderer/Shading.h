#pragma once

#include <handy/Guard.h>
#include <handy/tuple_utils.h>

#include <glad/glad.h>

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

inline void compileShader(const Shader & aShader, const char * aSource)
{
    glShaderSource(aShader, 1, &aSource, NULL);
    glCompileShader(aShader);

    handleGlslError(aShader,
                    GL_COMPILE_STATUS,
                    glGetShaderiv,
                    glGetShaderInfoLog);
}

Program makeLinkedProgram(std::initializer_list<std::pair<const GLenum/*stage*/,
                                                          const char * /*source*/>> aShaders);

} // namespace ad
