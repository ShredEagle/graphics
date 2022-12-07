#include "Shading.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <optional>
#include <regex>
#include <sstream>
#include <stack>

#include <cassert>

namespace ad {
namespace graphics {


namespace {

    std::ifstream openStream(std::filesystem::path aFile)
    {
        return std::ifstream{aFile};
    }

    // Only handle double-slash comments atm
    // TODO handle /* */ style comments
    void removeComments(std::string & aLine)
    {
        // Remove comments from "//" to end of the line
        aLine.resize(std::min(aLine.size(), aLine.find("//")));
    }

    const std::regex gInclusionRegex{R"#(#include\W*"(.+?)")#"};

    class FileLookup
    {
        using path = std::filesystem::path;

        class PopGuard : public std::ifstream
        {
        public:
            PopGuard(std::ifstream && aIn, std::stack<path> & aStack) :
                std::ifstream{std::move(aIn)},
                mStack{aStack}
            {}

            ~PopGuard() override
            {
                mStack.pop();
            }

        private:
            std::stack<path> & mStack;
        };

    public:
        FileLookup(const path & aRootFile)
        {
            mPathes.push(canonical(aRootFile));
        }
        
        std::pair<std::unique_ptr<PopGuard>, std::string> operator()(const std::string & aRelativePath)
        {
            path filePath = mPathes.top().parent_path() / aRelativePath;
            if (!is_regular_file(filePath))
            {
                throw ShaderCompilationError{
                    "GLSL inclusion error",
                    "Cannot include '" + filePath.make_preferred().string() 
                        + "', requested from '" + mPathes.top().string() + "'."};
            }
            // Cannot be called if the file does not exist
            filePath = canonical(filePath);
            mPathes.push(filePath);
            return {
                std::make_unique<PopGuard>(openStream(filePath), mPathes),
                filePath.string(),
            };
        }

        std::string top() const
        {
            return mPathes.top().string();
        }

    private:
        std::stack<path> mPathes;
    };

} // anonymous


SourceMap::Mapping ShaderSource::InclusionSourceMap::getLine(std::size_t aCompiledSourceLine) const
{
    const OriginalLine & original = mMap.at(aCompiledSourceLine);
    return {
        .mIdentifier = mIdentifiers[original.mIdentifierIndex],
        .mLine = original.mLineNumber,
    };
}


auto ShaderSource::InclusionSourceMap::registerSource(std::string_view aIdentifier) -> IdentifierId
{
    IdentifierId result = mIdentifiers.size();
    mIdentifiers.push_back(std::string{aIdentifier});
    return result;
}


void ShaderSource::InclusionSourceMap::associateLines(IdentifierId aIdentifier,
                                                      std::size_t aAssembledLine,
                                                      std::size_t aSourceLine)
{
    auto [iterator, didInsert] =
        mMap.emplace(aAssembledLine, OriginalLine{aIdentifier, aSourceLine});
    assert(didInsert);
}


ShaderSource::ShaderSource(std::string aSource, InclusionSourceMap aMap) :
    mSource{std::move(aSource)},
    mMap{std::move(aMap)}
{}


ShaderSource ShaderSource::Preprocess(std::filesystem::path aFile)
{
    // TODO handle parent_path changing when the included file is in a different directory
    // (i.e. the files it includes should not be rooted from the different directory)
    FileLookup lookup{aFile};
    return ShaderSource::Preprocess(openStream(aFile), lookup.top(), lookup);
}


ShaderSource ShaderSource::Preprocess(std::istream & aIn,
                                      const std::string & aIdentifier,
                                      const Lookup & aLookup)
{
    Assembled out;
    Preprocess_impl({aIn, aIdentifier}, out, aLookup);
    return ShaderSource{std::move(out.mStream.str()), std::move(out.mMapping)};
}


void ShaderSource::Preprocess_impl(Input aIn, Assembled & aOut, const Lookup & aLookup)
{
    std::size_t sourceLine = 0;
    InclusionSourceMap::IdentifierId sourceId = aOut.mMapping.registerSource(aIn.mIdentifier);
    for(std::string line; getline(aIn.mStream, line); )
    {
        ++sourceLine;
        removeComments(line);

        std::smatch matches;
        while(std::regex_search(line, matches, gInclusionRegex))
        {
            // Everything before the match is added to the output
            aOut.mStream << matches.prefix();

            // Recursive invocation on the included content
            std::pair<std::unique_ptr<std::istream>, std::string> included 
                = aLookup(matches[1]);
            Preprocess_impl({*included.first, included.second}, aOut, aLookup);

            // Everything up-after the include match is removed from the line
            // before the next evalution of the condition.
            line = matches.suffix();
        }
        // Everything on the line after the last match is added to the output
        // Yet we discard empty lines. This will notably discard the remaining empty line after an include.
        if(! line.empty())
        {
            // Note: Write a newline after last line, even if it did not contain one.
            //   This ensures that the characters in the parent file after the include are on a newline
            //   (TODO: Anyway, we probably should not support characters after an include directive)
            aOut.mStream << line << "\n";
            // Lines are 1-indexed, increment before writing to the map.
            aOut.mMapping.associateLines(sourceId, ++aOut.mOutputLine, sourceLine);
        }
    }
}


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
    // Diagnostic will receive the processed output.
    std::ostringstream diagnostic;
    if(auto errorLog =
            extractGlslError(aObjectId, GL_COMPILE_STATUS, glGetShaderiv, glGetShaderInfoLog))
    {
        try
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
            std::string errorLine;

            std::getline(log, errorLine); // following getline calls are in the inner while
            while(!errorLine.empty())
            {
                auto left = errorLine.find("(");
                auto right = errorLine.find(")");
                int column = std::stoi(errorLine.substr(0, left));    
                int line = std::stoi(errorLine.substr(left + 1, right - left));    
                assert(line > 0);

                // Assemble error message with the current body, 
                // plus all following lines that are indented.
                std::string errorMessage = errorLine.substr(right + 4) + "\n";
                while(std::getline(log, errorLine) && errorLine.starts_with("    "))
                {
                    errorMessage += errorLine + "\n";
                }

                // Once a non-indented line has been found, the error has been treated entirely.
                auto mapping = [&]() -> SourceMap::Mapping
                {
                     if (aSource.mMap != nullptr)
                     {
                        return aSource.mMap->getLine(line);
                     }
                     else
                     {
                        return {aSource.mIdentifier, (std::size_t)line};
                     }
                }();

                diagnostic << mapping.mIdentifier << " " << column  << "(line: " << mapping.mLine << ") : "
                    << sourceLines[line - 1] << "\n"
                    << "- " << errorMessage << "\n";
            }
        }
        catch(std::exception &aException)
        {
            diagnostic << *errorLog
                       << "Error log analysis failed with exception: " << aException.what();
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
