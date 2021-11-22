#include "SpriteSheet.h"

#include "detail/Json.h"


namespace ad {
namespace arte {


SpriteSheet::SpriteSheet(std::string aName, const filesystem::path & aSheetImage) :
    mName{std::move(aName)},
    mSheet{ImageRgba::LoadFile(aSheetImage, ImageOrientation::InvertVerticalAxis)}
{}


SpriteSheet SpriteSheet::LoadAseFile(const filesystem::path & aJsonData)
{
    std::ifstream jsonInput{aJsonData.string()};
    Json data;
    jsonInput >> data;
    
    const filesystem::path imagePath{data.at("meta").at("image").get<std::string>()};
    SpriteSheet spriteSheet{
        imagePath.stem().string(),
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
            frameJson.at("duration").get<Duration_t>(),
            std::move(area),
        });

        spriteSheet.mTotalDuration += spriteSheet.mFrames.back().duration;
    }

    return spriteSheet;
}


} // namespace arte
} // namespace ad
