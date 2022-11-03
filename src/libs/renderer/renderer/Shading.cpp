#include "Shading.h"

#include <algorithm>
#include <optional>
#include <sstream>

#include <cassert>

namespace ad {
namespace graphics {

Shader::Shader(GLenum aStage, ShaderSourceView aSource) : Shader(aStage)
{
    compileShader(*this, aSource);
}

std::optional<std::string> extractGlslError(GLuint objectId,
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
        infoLogGetter(objectId, maxLength, &maxLength, infoLog.data());

        // -1, because the returned log is null terminated.
        std::string errorLog(infoLog.begin(), infoLog.end() - 1);
        return {errorLog};
    }
    else
    {
        return std::nullopt;
    }
}


void handleCompilationError(GLuint aObjectId, ShaderSourceView aSource)
{
    if (auto errorLog = 
            extractGlslError(aObjectId, GL_COMPILE_STATUS, glGetShaderiv, glGetShaderInfoLog))
    {
        // Extract each line from the shader source code.
        // Disclaimer: this is stupid, so many string copies \o/
        // Some might argue that it somehow reflects the standard library (why can't you getLine on a string_view?)
        // TODO Replace with a vector of indices range for each line in the view.
        std::istringstream source{std::string{aSource}};
        std::string sourceLine;
        std::vector<std::string> sourceLines;
        while(std::getline(source, sourceLine))
        {
            sourceLines.push_back(sourceLine);
        }

        // Prepare a diagnostic for each error, notably showing the offending line.
        std::istringstream log{*errorLog};
        std::ostringstream diagnostic;
        std::string errorLine;
        while(std::getline(log, errorLine) && ! errorLine.empty())
        {
            auto left = errorLine.find("(");
            auto right = errorLine.find(")");
            int column = std::stoi(errorLine.substr(0, left));    
            int line = std::stoi(errorLine.substr(left + 1, right - left));    
            assert(line > 0);
            diagnostic << aSource.mIdentifier << " " << column  << "(" << line << ") : "
                << sourceLines[line - 1] << "\n"
                << errorLine.substr(right + 4) << "\n";
        }
        throw ShaderCompilationError("GLSL compilation error", "\n" + diagnostic.str());
    }
}

void handleLinkError(GLuint aObjectId)
{
    if (auto errorLog = 
            extractGlslError(aObjectId, GL_LINK_STATUS, glGetProgramiv, glGetProgramInfoLog))
    {
        throw ShaderCompilationError("GLSL link error", *errorLog);
    }
}

void compileShader(const Shader & aShader, ShaderSourceView aSource)
{
    const char * const data = aSource.data();
    const GLint size = static_cast<GLint>(aSource.size());
    glShaderSource(aShader, 1, &data, &size);
    glCompileShader(aShader);

    handleCompilationError(aShader, aSource);
}

Program makeLinkedProgram(std::initializer_list<std::pair<const GLenum/*stage*/,
                                                          ShaderSourceView /*source*/>> aShaders)
{
    Program program;

    std::vector<Shader> attached;
    for (const auto & pair : aShaders)
    {
        attached.emplace_back(pair.first, pair.second);
        glAttachShader(program, attached.back());
    }

    glLinkProgram(program);
    handleLinkError(program);

    // Apparently, it is a good practice to detach as soon as link is done
    std::for_each(attached.begin(), attached.end(), [&program](const Shader & shader)
    {
        glDetachShader(program, shader);
    });

    return program;
}

} // namespace graphics
} // namespace ad
