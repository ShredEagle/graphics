#pragma once

#include <handy/Guard.h>

#include "gl_helpers.h"
#include "GL_Loader.h"


namespace ad {
namespace graphics {


// Defined in ShaderSource.h
class ShaderSource;
struct SourceMap;


/// \brief Augments the (char *) shader source code (as a string_view) with
/// (optional) way to map the line in error with an identifier.
///
/// The identifier might be a fixed string, or a mapping constructed by preprocessing includes.
struct ShaderSourceView
{
    /*implicit*/ ShaderSourceView(const std::string & aSource, std::string aIdentifier = "") :
        mSource{aSource},
        mIdentifier{std::move(aIdentifier)}
    {}

    /*implicit*/ ShaderSourceView(const char * aSource, std::string aIdentifier = "") :
        mSource{aSource},
        mIdentifier{std::move(aIdentifier)}
    {}

    /*implicit*/ ShaderSourceView(const ShaderSource & aShaderSource);

    operator std::string_view () const
    { return mSource; }

    auto data() const
    { return mSource.data(); }

    auto size() const
    { return mSource.size(); }

    std::string_view mSource;
    // Implementation note:
    //   For simplicity, host a string directly that will be used if there is no SourceMap.
    //   An alternative would be to specialize SourceMap, but I do not know where to host the instance.
    std::string mIdentifier;
    const SourceMap * mMap = nullptr;
};


struct [[nodiscard]] Shader : public ResourceGuard<GLuint>
{
    Shader(GLenum aStage) :
        ResourceGuard<GLuint>{glCreateShader(aStage), glDeleteShader},
        mStage(aStage)
    {}

    Shader(GLenum aStage, ShaderSourceView aSource);

    GLenum mStage;
};


struct [[nodiscard]] Program : public ResourceGuard<GLuint>
{
    Program() :
        ResourceGuard<GLuint>{glCreateProgram(), glDeleteProgram}
    {}
};


inline void use(const Program & aProgram)
{
    glUseProgram(aProgram);
}


inline void use(Name<Program> aProgram)
{
    glUseProgram(aProgram);
}


inline void disableProgram()
{
    glUseProgram(0);
}


/// \note Provided for ability to use bind_guard
inline void bind(const Program & aProgram)
{
    use(aProgram);
}


inline void bind(Name<Program> aProgram)
{
    use(aProgram);
}


/// \note Provided for ability to use bind_guard
inline void unbind(const Program &)
{
    disableProgram();
}


inline Name<Program> getBound(const Program & aBuffer)
{
    GLint current;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current);
    return Name<Program>{(GLuint)current, typename Name<Program>::UnsafeTag{}};
}


void handleCompilationError(GLuint aObjectId, ShaderSourceView aSource);

void handleLinkError(GLuint aObjectId);


/// \brief User defined exception to signal an error when compiling a shader.
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

void compileShader(const Shader & aShader, ShaderSourceView aSource);

//Program makeLinkedProgram(std::initializer_list<std::pair<const GLenum/*stage*/,
//                                                          const char * /*source*/>> aShaders);

Program makeLinkedProgram(std::initializer_list<std::pair<const GLenum/*stage*/,
                                                          ShaderSourceView /*source*/>> aShaders);


} // namespace graphics
} // namespace ad