#include "DebugDrawer.h"

namespace ad {
namespace gltfviewer {


void DebugDrawer::addLine(graphics::DrawLine::Line aLine)
{
    mDrawLine.addLine(aLine);
}


void DebugDrawer::render() const
{
    mDrawLine.render();
    mDrawLine.clearShapes();
}
    
    
} // namespace gltfviewer
} // namespace ad
