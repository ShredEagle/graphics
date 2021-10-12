#pragma once


#include <graphics/Curving.h>
#include <graphics/Timer.h>

#include <graphics/AppInterface.h>

#include <math/Transformations.h>
#include <math/VectorUtilities.h>

// Dirty include, to get the GLFW input definitions
#include <GLFW/glfw3.h>


namespace ad {
namespace graphics {


constexpr GLfloat gInitialHalfWidth = 0.05f;


math::Box<GLfloat> getViewVolume(Size2<int> aRenderResolution,
                                 GLfloat aWindowHeight = 3.,   
                                 GLfloat aNear = 10.,
                                 GLfloat aDepth = 20.)
{
    math::Size<3, GLfloat> frontSize{
        math::makeSizeFromHeight(aWindowHeight, math::getRatio<GLfloat>(aRenderResolution)),
        aDepth
    };

    return math::Box<GLfloat>{
        math::Position<3, GLfloat>{
            -frontSize.width() / 2.f,
            -frontSize.height() / 2.f,
            aNear},
        frontSize,
    };
}

class Scene
{
public:
    Scene(Size2<int> aRenderResolution, std::shared_ptr<AppInterface> aAppInterface) :
        mViewVolume{ getViewVolume(aRenderResolution) },
        mWindowToView{
            math::trans2d::window(
                math::Rectangle<GLfloat>{ {0.f, 0.f}, static_cast<math::Size<2, GLfloat>>(aRenderResolution) },
                mViewVolume.frontRectangle())
        },
        mCurving{ 
            gCurveSubdivisions,
            math::trans3d::orthographicProjection(mViewVolume)
        },
        mCurves{
            Curving::Instance{ 
                math::Bezier<4, 3, GLfloat>{
                    math::Position<3, GLfloat>{-1.5f, -0.5f, 0.f},
                    math::Position<3, GLfloat>{-0.3f,  0.5f, 0.f},
                    math::Position<3, GLfloat>{ 0.3f, -0.5f, 0.f},
                    math::Position<3, GLfloat>{ 0.5f,  0.5f, 0.f}
                },
                gInitialHalfWidth,
                gInitialHalfWidth
            },
            { 
                math::Bezier<4, 3, GLfloat>{
                    math::Position<3, GLfloat>{ 0.5f,  0.5f, 0.f},
                    math::Position<3, GLfloat>{ 0.7f,  1.5f, 0.f},
                    math::Position<3, GLfloat>{ 1.0f,  1.0f, 0.f},
                    math::Position<3, GLfloat>{ 1.5f,  0.5f, 0.f}
                },
                gInitialHalfWidth,
                gInitialHalfWidth
            },
        }
    {
        using namespace std::placeholders;
        aAppInterface->registerMouseButtonCallback(std::bind(&Scene::callbackMouseButton, this, _1, _2, _3, _4, _5));
        aAppInterface->registerCursorPositionCallback(std::bind(&Scene::callbackCursorPosition, this, _1, _2));
        aAppInterface->registerKeyCallback(std::bind(&Scene::callbackKey, this, _1, _2, _3, _4));
    }

    void step(const Timer & aTimer)
    {
        static constexpr GLfloat gCyclesPerSecond = 0.2f;
        static constexpr GLfloat gAmplitude = 4.f;
        GLfloat t = (std::cos(aTimer.time() * 2 * math::pi<GLfloat> * gCyclesPerSecond) + 1.f) / 2.f;

        // Pumping the middle point
        if (mBeating)
        {
            mCurves.at(0).endHalfWidth = mCurves.at(1).startHalfWidth = 0.02 * (1 + gAmplitude * t);
        }

        if (mRotate)
        {
            mAngle += math::Radian<GLfloat>{(GLfloat)aTimer.delta()};
        }
        mCurves.at(0).modelTransform = mCurves.at(1).modelTransform = math::trans3d::rotateY(mAngle);
    }

    void render()
    {
        mCurving.render(mCurves);
    }

private:
    void placeOutHandle(math::Position<3, GLfloat> aPosition)
    {
        auto midpoint = mCurves.at(1).bezier[0];
        mCurves.at(1).bezier[1] = aPosition;
        mCurves.at(0).bezier[2] = midpoint + (midpoint - aPosition);
    }

    void callbackMouseButton(int button, int action, int mods, double xpos, double ypos)
    {
        // homogenenous position
        auto mouseViewPosition = 
            math::Position<3, GLfloat>{ (GLfloat)xpos, (GLfloat)ypos, 1.0f } * mWindowToView;

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            mHandleClicked = true;
        }
        else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        {
            mHandleClicked = false;
        }
    }

    void callbackCursorPosition(double xpos, double ypos)
    {
        if (mHandleClicked)
        {
            auto mouseViewPosition = 
                math::Position<3, GLfloat>{ (GLfloat)xpos, (GLfloat)ypos, 1.0f } * mWindowToView;

            // IMPORTANT: Y is negated (because ypos is incrementing toward the bottom of the window)
            placeOutHandle({mouseViewPosition.x(), -mouseViewPosition.y(), 0.f});
        }
    }

    void callbackKey(int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
        {
            mRotate = !mRotate;
        }
        else if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS)
        {
            mAngle = math::Radian<GLfloat>{0.};
        }
        else if (key == GLFW_KEY_B && action == GLFW_PRESS)
        {
            if (!(mBeating = !mBeating))
            {
                mCurves.at(0).endHalfWidth = mCurves.at(1).startHalfWidth = gInitialHalfWidth;
            }
        }
        else if (key == GLFW_KEY_F && action == GLFW_PRESS)
        {
            if (mWireframe = !mWireframe)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }
    }

    static constexpr GLsizei gCurveSubdivisions{75};

    math::Box<GLfloat> mViewVolume;
    math::AffineMatrix<3, GLfloat> mWindowToView;
    Curving mCurving;
    std::vector<Curving::Instance> mCurves;

    bool mHandleClicked{false};
    bool mWireframe{false};
    bool mBeating{true};
    bool mRotate{false};
    math::Radian<GLfloat> mAngle{0.};
};


} // namespace graphics
} // namespace ad
