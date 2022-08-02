#include "SpriteSheet.h"

#include "detail/Json.h"

#include <math/Range.h>

#include <fstream>
#include <sstream>


namespace ad {
namespace arte {


AnimationSpriteSheet AnimationSpriteSheet::LoadAseFile(const filesystem::path & aJsonData)
{
    std::ifstream jsonInput{aJsonData.string()};
    Json data;
    jsonInput >> data;
    
    const filesystem::path imagePath{data.at("meta").at("image").get<std::string>()};
    AnimationSpriteSheet spriteSheet{
        aJsonData.stem().string(),
        aJsonData.parent_path() / imagePath
    };
    spriteSheet.mScale = std::stof(data.at("meta").at("scale").get<std::string>());

    // Frames
    for (auto frameJson : data.at("frames"))
    {
        SpriteArea area{
            {frameJson.at("frame").at("x").get<int>(), frameJson.at("frame").at("y").get<int>()},
            {frameJson.at("frame").at("w").get<int>(), frameJson.at("frame").at("h").get<int>()},
        };
        spriteSheet.mFrames.push_back({
            frameJson.at("filename").get<std::string>(),
            std::move(area),
            frameJson.at("duration").get<Duration_t>(),
        });

        spriteSheet.mTotalDuration += spriteSheet.mFrames.back().duration;
    }

    return spriteSheet;
}


TileSheet TileSheet::LoadMetaFile(const filesystem::path & aJsonData)
{
    std::ifstream jsonInput{aJsonData.string()};
    Json data;
    jsonInput >> data;

    const filesystem::path imagePath{data.at("file").get<std::string>()};
    TileSheet spriteSheet{
        aJsonData.stem().string(),
        aJsonData.parent_path() / imagePath
    };
    spriteSheet.mScale = data.at("scale").get<float>();

    const std::string prefix = data["set"]["prefix"];

    auto grid = data["set"]["regularGrid"];
    math::Size<2, int> dimension{grid["width"], grid["height"]};
    math::Vec<2, int> tileOffset = 
        dimension.as<math::Vec>() + math::Vec<2, int>{grid["xBorder"], grid["yBorder"]};
    math::Position<2, int> startPosition{grid["xOffset"], grid["yOffset"]};

    for (int row : math::Range<int>{grid["yCount"]})
    {
        for (int column : math::Range<int>{grid["xCount"]})
        {
            std::ostringstream nameOss;
            nameOss << prefix << column << "_" << row;

            spriteSheet.mFrames.push_back({
                nameOss.str(),
                {startPosition + tileOffset.cwMul({column, row}), dimension}
            });
        }
    }
    return spriteSheet;
}


} // namespace arte
} // namespace ad
