#pragma once


#include "Logging.h"
#include "Polar.h"

#include <graphics/AppInterface.h>
#include <graphics/CameraUtilities.h>

#include <renderer/GL_Loader.h>

// Dirty include, to get the GLFW input definitions
#include <GLFW/glfw3.h>


namespace ad {
namespace gltfviewer {


class UserCamera
{
public:
    UserCamera(std::shared_ptr<graphics::AppInterface> aAppInterface) :
        mAppInterface{std::move(aAppInterface)}
    {}

    math::AffineMatrix<4, GLfloat> update();

    void callbackCursorPosition(double xpos, double ypos);
    void callbackMouseButton(int button, int action, int mods, double xpos, double ypos);

    void setOrigin(math::Position<3, GLfloat> aNewOrigin)
    { mPolarOrigin = aNewOrigin; }

    math::Matrix<4, 4, float> setViewedHeight(GLfloat aHeight);
    math::Matrix<4, 4, float> multiplyViewedHeight(GLfloat aFactor);

private:
    enum class ControlMode
    {
        None,
        Orbit,
        Pan,
    };

    std::shared_ptr<graphics::AppInterface> mAppInterface;
    Polar mPosition{3.f};
    math::Position<3, GLfloat> mPolarOrigin{0.f, 0.f, 0.f};
    ControlMode mControlMode;
    math::Position<2, GLfloat> mPreviousDragPosition{0.f, 0.f};
    GLfloat mCurrentProjectionHeight;

    static constexpr math::Position<3, GLfloat> gGazePoint{0.f, 0.f, 0.f};
    static constexpr math::Vec<2, GLfloat> gMouseControlFactor{1/700.f, 1/700.f};
    static constexpr GLfloat gViewedDepth = 10000;
};


inline math::AffineMatrix<4, GLfloat> UserCamera::update()
{
    const math::Position<3, GLfloat> cameraCartesian = mPosition.toCartesian();
    ADLOG(gDrawLogger, trace)("Camera position {}.", cameraCartesian);

    math::Vec<3, GLfloat> gazeDirection = gGazePoint - cameraCartesian;
    return graphics::getCameraTransform(mPolarOrigin + cameraCartesian.as<math::Vec>(),
                                        gazeDirection,
                                        mPosition.getUpTangent());
}


inline void UserCamera::callbackCursorPosition(double xpos, double ypos)
{
    using Radian = math::Radian<GLfloat>;
    math::Position<2, GLfloat> cursorPosition{(GLfloat)xpos, (GLfloat)ypos};

    // top-left corner origin
    switch (mControlMode)
    {
    case ControlMode::Orbit:
    {
        auto angularIncrements = (cursorPosition - mPreviousDragPosition).cwMul(gMouseControlFactor);

        // The viewed object should turn in the direction of the mouse,
        // so the camera angles are changed in the opposite direction (hence the substractions).
        mPosition.azimuthal -= Radian{angularIncrements.x()};
        mPosition.polar -= Radian{angularIncrements.y()};
        mPosition.polar = std::max(Radian{0}, std::min(Radian{math::pi<GLfloat>}, mPosition.polar));
        break;
    }
    case ControlMode::Pan:
    {
        auto dragVector{cursorPosition - mPreviousDragPosition};
        dragVector *= mCurrentProjectionHeight / mAppInterface->getWindowSize().height();
        mPolarOrigin -= dragVector.x() * mPosition.getCCWTangent().normalize() 
                        - dragVector.y() * mPosition.getUpTangent().normalize();
        break;
    }
    }

    mPreviousDragPosition = cursorPosition;
}


inline void UserCamera::callbackMouseButton(int button, int action, int mods, double xpos, double ypos)
{
    if (action == GLFW_PRESS)
    {
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_LEFT:
            mControlMode = ControlMode::Orbit;
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            mControlMode = ControlMode::Pan;
            break;
        }
    }
    else if ((button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_MIDDLE)
             && action == GLFW_RELEASE)
    {
        mControlMode = ControlMode::None;
    }
}


inline math::Matrix<4, 4, float> UserCamera::setViewedHeight(GLfloat aHeight)
{
    mCurrentProjectionHeight = aHeight;
    const math::Box<GLfloat> projectedBox =
        graphics::getViewVolumeRightHanded(mAppInterface->getWindowSize(), mCurrentProjectionHeight, gViewedDepth, 2*gViewedDepth);
    math::Matrix<4, 4, float> projectionTransform = 
        math::trans3d::orthographicProjection(projectedBox)
        * math::trans3d::scale(1.f, 1.f, -1.f); // OpenGL clipping space is left handed.

    return projectionTransform;
}


inline math::Matrix<4, 4, float> UserCamera::multiplyViewedHeight(GLfloat aFactor)
{
    return setViewedHeight(mCurrentProjectionHeight * aFactor);
}


} // namespace gltfviewer
} // namespace ad
