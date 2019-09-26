#include "tiles.h"

#include "json.h"

#include <resource/PathProvider.h>

#include <Math/Range.h>

#include <sstream>


namespace ad {
namespace dataformat {

SpriteSheet loadMeta(std::istream & aDatastream)
{
    json content;
    aDatastream >> content;

    SpriteSheet sheet{
        {},
        Image{pathFor(content["file"])}
    };
    
    const std::string prefix = content["set"]["prefix"];

    auto grid = content["set"]["regularGrid"];
    Size2<int> dimension{grid["width"], grid["height"]};
    Vec2<int> tileOffset = static_cast<Vec2<int>>(dimension) 
                           + Vec2<int>{grid["xBorder"], grid["yBorder"]};
    Vec2<int> startOffset = Vec2<int>{grid["xOffset"], grid["yOffset"]};

    for (int row : math::Range<int>{grid["yCount"]})
    {
        for (int column : math::Range<int>{grid["xCount"]})
        {
            std::ostringstream nameOss;
            nameOss << prefix << column << "_" << row;

            sheet.mSprites.push_back({nameOss.str(),
                                      startOffset + tileOffset.hadamard({column, row}),
                                      dimension});
        }
    }

    return sheet;
}

}} // namespace ad::dataformat
