#pragma once

#include <handy/Guard.h>
#include <handy/tuple_utils.h>

#include "GL_Loader.h"

// TODO Simplify the inclusions, while moving out ShaderSource.
#include <filesystem>
#include <functional>
#include <map>
#include <stack>
#include <vector>

namespace ad {
namespace graphics {

struct SourceMap
{
    struct Mapping
    {
        std::string mIdentifier;
        std::size_t mLine;
    };

    virtual Mapping getLine(std::size_t aCompiledSourceLine) const = 0;
};


/// \brief Host the shader code string, and offer preprocessing for #include
class ShaderSource
{
    friend struct ShaderSourceView;

    class InclusionSourceMap : public SourceMap
    {
    public:
        using IdentifierId = std::size_t;

        SourceMap::Mapping getLine(std::size_t aCompiledSourceLine) const override;

        IdentifierId registerSource(std::string_view aIdentifier);
        void associateLines(IdentifierId aIdentifier,
                            std::size_t aAssembledLine,
                            std::size_t aSourceLine);
        
    private:
        struct OriginalLine
        {
            std::size_t mIdentifierIndex;
            std::size_t mLineNumber;
        };

        std::vector<std::string> mIdentifiers;
        std::map<std::size_t/*line in assembled output*/, OriginalLine> mMap;
    };

public:
    /// \brief The lookup function takes the string as found inside the include directive quotes,
    /// and returns (unique pointer to) the opened stream and the identifier for the included content.
    /// \note In the case of including files in nested subfolders, the returned identifier could be the full path
    /// whereas the input (from the include directive) might be a relative path from the current folder.
    using Lookup = std::function<
        std::pair<std::unique_ptr<std::istream>, std::string/*identifier*/>(const std::string &)>;

    static ShaderSource Preprocess(std::istream & aIn,
                                   const std::string & aIdentifier,
                                   const Lookup & aLookup);

    static ShaderSource Preprocess(std::istream && aIn,
                                   const std::string & aIdentifier,
                                   const Lookup & aLookup)
    { return Preprocess(aIn, aIdentifier, aLookup); }

    static ShaderSource Preprocess(std::filesystem::path aFile);

    const InclusionSourceMap & getSourceMap()
    { return mMapping; }

private:
    struct Input
    {
        std::istream & mStream;
        std::string_view mIdentifier;
    };

    struct Assembled
    {
        std::ostringstream mStream;
        std::size_t mOutputLine{0};
        InclusionSourceMap mMapping;
    };

    static void Preprocess_impl(Input aIn, Assembled & aOut, const Lookup & aLookup);

    ShaderSource(std::string aSource, InclusionSourceMap aMapping);

    std::string mSource;
    InclusionSourceMap mMapping;
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
