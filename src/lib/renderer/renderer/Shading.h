#pragma once

#include <glad/glad.h>

#include <functional>

namespace ad
{

template <int N_stage>
struct [[nodiscard]] Shader
{
    // Disable copy
    Shader(const Shader &) = delete;
    Shader & operator=(const Shader &) = delete;

    // Movable
    Shader(Shader && aOther) :
        mShaderId{aOther.mShaderId}
    {
        aOther.mShaderId = 0;
    }

    Shader(GLuint aShaderId) :
        mShaderId(aShaderId)
    {}

    ~Shader()
    {
        glDeleteShader(mShaderId);
    }

    operator GLuint ()
    { return mShaderId; }

    GLuint mShaderId;
};


struct [[nodiscard]] Program
{
    // Disable copy
    Program(const Program &) = delete;
    Program & operator=(const Program &) = delete;

    // Movable
    Program(Program && aOther) :
        mProgramId{aOther.mProgramId}
    {
        aOther.mProgramId = 0;
    }

    Program(GLuint aProgramId) :
        mProgramId(aProgramId)
    {}

    ~Program()
    {
        glDeleteProgram(mProgramId);
    }

    operator GLuint ()
    { return mProgramId; }

    GLuint mProgramId;
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
Shader<N_stage> compileShader(const std::string &aSource)
{
    Shader<N_stage> shader{glCreateShader(N_stage)};

    //glShadersource(shader, 1, char*[]{aSource.data()}, NULL);
    const char* sourceProxy = aSource.data(); 
    glShaderSource(shader, 1, &sourceProxy, NULL);
    glCompileShader(shader);

    handleGlslError(shader,
                    GL_COMPILE_STATUS,
                    glGetShaderiv,
                    glGetShaderInfoLog);

    return shader;
}

} // namespace ad
