#pragma once


#include "AppInterface.h"
#include "commons.h"

#include <math/Homogeneous.h>

#include <renderer/Drawing.h>


namespace ad {
namespace graphics {


class DrawLine
{
public:
    struct Line;
private:
    using InstanceData = std::vector<Line>;

public:
    DrawLine(std::shared_ptr<AppInterface> aAppInterface);

    /// \brief Remove all shapes that were previously added.
    ///
    /// Should be called between each frame to implement animation.
    void clearShapes();

    void addLine(Line aLine);

    /// \brief Render all shapes that were added since the last call to `clearShapes()`.
    void render() const;

    void setCameraTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation);
    void setProjectionTransformation(const math::Matrix<4, 4, GLfloat> & aTransformation);

private:
    void setWindowResolution(Size2<int> aNewResolution);

private:
    DrawContext mDrawContext;
    InstanceData mInstances;
    std::shared_ptr<AppInterface::SizeListener> mListenWindowSize;
};


struct DrawLine::Line
{
    Position3<GLfloat> mOrigin;
    Position3<GLfloat> mEnd;
    GLfloat mWidth_screen{2.f};
    Color mColor{math::sdr::gWhite};
};

} // namespace graphics
} // namespace ad
