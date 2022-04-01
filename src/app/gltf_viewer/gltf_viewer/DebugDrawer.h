#pragma once


#include <graphics/DrawLine.h>


namespace ad {
namespace gltfviewer {


/// \brief Immediate mode debug drawer
class DebugDrawer
{
public:
    DebugDrawer(const std::shared_ptr<graphics::AppInterface> & aAppInterface) :
        mDrawLine{aAppInterface}
    {}

    void addLine(graphics::DrawLine::Line aLine);

    void render() const;

private:
    // TODO unmute
    mutable graphics::DrawLine mDrawLine;
};

} // namespace gltfviewer
} // namespace ad
