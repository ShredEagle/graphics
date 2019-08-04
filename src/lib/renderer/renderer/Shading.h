#pragma once

#include <handy/Guard.h>

#include <glad/glad.h>

#include <functional>

namespace ad
{

template <int N_stage>
struct [[nodiscard]] Shader : public ResourceGuard<GLuint>
{
    Shader() :
        ResourceGuard<GLuint>{glCreateShader(N_stage), glDeleteShader}
    {}
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

template <int N_stage>
void compileShader(const Shader<N_stage> & aShader, const std::string &aSource)
{
    //glShadersource(shader, 1, char*[]{aSource.data()}, NULL);
    const char* sourceProxy = aSource.data(); 
    glShaderSource(aShader, 1, &sourceProxy, NULL);
    glCompileShader(aShader);

    handleGlslError(aShader,
                    GL_COMPILE_STATUS,
                    glGetShaderiv,
                    glGetShaderInfoLog);
}

} // namespace ad
