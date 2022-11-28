#pragma once

#include <handy/Guard.h>
#include <handy/tuple_utils.h>

#include "GL_Loader.h"

#include <filesystem>
#include <functional>
#include <vector>

namespace ad {
namespace graphics {


/// \brief Host the shader code string, and offer preprocessing for #include
class ShaderSource
{
    friend struct ShaderSourceView;

public:
    using Lookup = std::function<std::unique_ptr<std::istream>(const std::string &)>;

    static ShaderSource Preprocess(std::istream & aIn, const Lookup & aLookup);
    static ShaderSource Preprocess(std::istream && aIn, const Lookup & aLookup)
    { return Preprocess(aIn, aLookup); }

    static ShaderSource Preprocess(std::filesystem::path aFile);

private:
    static void Preprocess_impl(std::istream & aIn, std::ostream & aOut, const Lookup & aLookup);

    ShaderSource(std::string aSource);

    std::string mSource;
};


/// \brief Augments the (char *) shader source code (as a string_view) with an
/// optional name helping to identify the shader associated to the source.
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

    /*implicit*/ ShaderSourceView(const ShaderSource & aShaderSource) :
        mSource{aShaderSource.mSource}
        // TODO handle some form of identifier
    {}

    operator std::string_view () const
    { return mSource; }

    auto data() const
    { return mSource.data(); }

    auto size() const
    { return mSource.size(); }

    std::string_view mSource;
    std::string mIdentifier{""};
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


inline void disableProgram()
{
    glUseProgram(0);
}


/// \note Provided for ability to use bind_guard
inline void bind(const Program & aProgram)
{
    use(aProgram);
}

/// \note Provided for ability to use bind_guard
inline void unbind(const Program &)
{
    disableProgram();
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
