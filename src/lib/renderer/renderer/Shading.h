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

inline void handleGlslError(GLuint objectId,
                     GLenum aStatusEnumerator,
                     std::function<void(GLuint, GLenum, GLint*)> statusGetter,
                     std::function<void(GLuint, GLsizei, GLsizei*, GLchar*)> infoLogGetter)
{
    GLint status;
    statusGetter(objectId, aStatusEnumerator, &status);
    if(status == GL_FALSE)
    {
        GLint maxLength = 0;
        statusGetter(objectId, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        infoLogGetter(objectId, maxLength, &maxLength, &infoLog[0]);

        std::string errorLog(infoLog.begin(), infoLog.end());
        std::cerr << "GLSL error[" << maxLength << "]: " << errorLog;

        throw std::runtime_error("GLSL error");
    }
}

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
