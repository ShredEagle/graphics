#include "Shading.h"

#include <algorithm>

namespace ad {

Shader::Shader(GLenum aStage, const char * aSource) : Shader(aStage)
{
    compileShader(*this, aSource);
}

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

        throw ShaderCompilationError("GLSL error", errorLog);
    }
}


Program makeLinkedProgram(std::initializer_list<std::pair<const GLenum/*stage*/,
                                                          const char * /*source*/>> aShaders)
{
    Program program;

    std::vector<Shader> attached;
    for (const auto & pair : aShaders)
    {
        attached.emplace_back(pair.first, pair.second);
        //Shader shader{pair.first, pair.second};
        glAttachShader(program, attached.back());
        //attached.push_back(shader);
    }

    glLinkProgram(program);
    handleGlslError(program, GL_LINK_STATUS, glGetProgramiv, glGetProgramInfoLog);

    // Apparently, it is a good practice to detach as soon as link is done
    std::for_each(attached.begin(), attached.end(), [&program](const Shader & shader)
    {
        glDetachShader(program, shader);
    });

    return program;
}

} // namespace ad
