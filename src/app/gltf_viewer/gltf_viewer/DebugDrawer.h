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

    void setCameraTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation);
    void setProjectionTransformation(const math::Matrix<4, 4, GLfloat> & aTransformation);

private:
    // TODO unmute
    mutable graphics::DrawLine mDrawLine;
};

} // namespace gltfviewer
} // namespace ad
