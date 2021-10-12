#include "tiles.h"

#include "json.h"

#include <resource/PathProvider.h>

#include <math/Range.h>

#include <sstream>


namespace ad {
namespace graphics {
namespace dataformat {

SpriteSheet loadMeta(std::istream & aDatastream)
{
    json content;
    aDatastream >> content;

    SpriteSheet sheet{
        {},
        Image{pathFor(content.at("file").get<std::string>()).string()}
    };

    const std::string prefix = content["set"]["prefix"];

    auto grid = content["set"]["regularGrid"];
    Size2<int> dimension{grid["width"], grid["height"]};
    Vec2<int> tileOffset = static_cast<Vec2<int>>(dimension)
                           + Vec2<int>{grid["xBorder"], grid["yBorder"]};
    Position2<int> startPosition{grid["xOffset"], grid["yOffset"]};

    for (int row : math::Range<int>{grid["yCount"]})
    {
        for (int column : math::Range<int>{grid["xCount"]})
        {
            std::ostringstream nameOss;
            nameOss << prefix << column << "_" << row;

            sheet.mSprites.push_back({nameOss.str(),
                                      startPosition + tileOffset.cwMul({column, row}),
                                      dimension});
        }
    }

    return sheet;
}

} // namespace dataformat
} // namespace graphics
} // namespace ad
