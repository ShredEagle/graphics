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
    

void DebugDrawer::setCameraTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation)
{
    mDrawLine.setCameraTransformation(aTransformation);
}


void DebugDrawer::setProjectionTransformation(const math::Matrix<4, 4, GLfloat> & aTransformation)
{
    mDrawLine.setProjectionTransformation(aTransformation);
}

    
} // namespace gltfviewer
} // namespace ad
