#include "ShaderSource.h"

#include "Shading.h"

#include "utilities/FileLookup.h"

#include <fstream>
#include <regex>
#include <sstream>

#include <cassert>


namespace ad {
namespace graphics {


namespace {

    // Only handle double-slash comments atm
    // TODO handle /* */ style comments
    void removeComments(std::string & aLine)
    {
        // Remove comments from "//" to end of the line
        aLine.resize(std::min(aLine.size(), aLine.find("//")));
    }

    const std::regex gInclusionRegex{R"#(#include\W*"(.+?)")#"};

} // anonymous


SourceMap::Mapping ShaderSource::InclusionSourceMap::getLine(std::size_t aCompiledSourceLine) const
{
    // The source lines are 1-indexed in error messages, but mMap is 0-indexed.
    const OriginalLine & original = mMap.at(aCompiledSourceLine - 1);
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


void ShaderSource::InclusionSourceMap::addLineOrigin(IdentifierId aOrigin,
                                                     std::size_t aLineNumber)
{
    mMap.push_back(OriginalLine{aOrigin, aLineNumber});
}


ShaderSource::ShaderSource(std::string aSource, InclusionSourceMap aMap) :
    mSource{std::move(aSource)},
    mMap{std::move(aMap)}
{}


ShaderSource ShaderSource::Preprocess(std::filesystem::path aFile,
                                      const Defines & aMacros)
{
    // TODO handle parent_path changing when the included file is in a different directory
    // (i.e. the files it includes should not be rooted from the different directory)
    FileLookup lookup{aFile};
    return ShaderSource::Preprocess(std::ifstream{aFile}, aMacros, lookup.top(), lookup);
}


ShaderSource ShaderSource::Preprocess(std::istream & aIn,
                                      const Defines & aMacros,
                                      const std::string & aIdentifier,
                                      const Lookup & aLookup)
{
    Assembled out;
    Preprocess_impl({aIn, aMacros, aIdentifier}, out, aLookup);
    return ShaderSource{out.mStream.str(), std::move(out.mMap)};
}


void ShaderSource::Preprocess_impl(Input aIn, Assembled & aOut, const Lookup & aLookup)
{
    std::size_t sourceLine = 0;
    InclusionSourceMap::IdentifierId sourceId = aOut.mMap.registerSource(aIn.mIdentifier);

    // Only do the special-case handling of first line if there are macros
    // * it is not useful in other situations
    // * most importantly, only the top-level file should start with the #version directive 
    //   (and we do not forward the macro to nested includes)
    if(!aIn.mMacros.empty())
    {
        // Advance past the first non-empty line, which has to be the #version directive
        for(std::string line; getline(aIn.mStream, line);)
        {
            // Lines are 1-indexed, increment before writing to the map.
            ++sourceLine;
            removeComments(line);

            if(!line.empty())
            {
                if(!line.starts_with("#version"))
                {
                    // '#version' is not the first directive in shader
                    assert(false);
                    // In non-debug builds, let it escape, the error will be caught at shader compilation.
                }
                aOut.mStream << line << "\n";
                aOut.mMap.addLineOrigin(sourceId, sourceLine);
                break;
            }
        }

        // Define macros
        for (const auto & macro : aIn.mMacros)
        {
            // IMPORTANT: Does not increment sourceLine, as it is the source line number in the current file.
            // (Injecting those defines do not advance in the current file.)
            // Yet it still needs to add a line origin, as it keeps track of the line number in the output source code.
            aOut.mStream << "#define " << macro << "\n";
            aOut.mMap.addLineOrigin(InclusionSourceMap::gInternalSource, 0);
        }
    }

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
            // Important: do **not** forward macros.
            //   They only need to be defined at the top level, and it prevents to test for #version in nested files.
            Preprocess_impl({*included.first, Defines{}, included.second}, aOut, aLookup);

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
            aOut.mMap.addLineOrigin(sourceId, sourceLine);
        }
    }
}


} // namespace graphics
} // namespace ad