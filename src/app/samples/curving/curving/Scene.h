#pragma once


#include <engine/Curving.h>
#include <engine/Timer.h>

#include <engine/Engine.h>

#include <math/Transformations.h>
#include <math/VectorUtilities.h>

// Dirty include, to get the GLFW input definitions
#include <GLFW/glfw3.h>


namespace ad
{


math::Box<GLfloat> getViewVolume(Size2<int> aRenderResolution,
                                 GLfloat aWindowHeight = 3.,   
                                 GLfloat aNear = 0.,
                                 GLfloat aDepth = 100.)
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
    Scene(Size2<int> aRenderResolution, std::shared_ptr<Engine> aEngine) :
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
                0.04f,
                0.02f
            },
            { 
                math::Bezier<4, 3, GLfloat>{
                    math::Position<3, GLfloat>{ 0.5f,  0.5f, 0.f},
                    math::Position<3, GLfloat>{ 0.7f,  1.5f, 0.f},
                    math::Position<3, GLfloat>{ 1.0f,  1.0f, 0.f},
                    math::Position<3, GLfloat>{ 1.5f,  0.5f, 0.f}
                },
                0.02f,
                0.04f
            },
        }
    {
        using namespace std::placeholders;
        aEngine->registerMouseButtonCallback(std::bind(&Scene::callbackMouseButton, this, _1, _2, _3, _4, _5));
        aEngine->registerCursorPositionCallback(std::bind(&Scene::callbackCursorPosition, this, _1, _2));
    }

    void step(const Timer & aTimer)
    {
        static constexpr GLfloat gCyclesPerSecond = 0.8f;
        static constexpr GLfloat gAmplitude = 4.f;
        GLfloat t = (std::cos(aTimer.time() * 2 * math::pi<GLfloat> * gCyclesPerSecond) + 1.f) / 2.f;

        // Pumping the middle point
        mCurves.at(0).endHalfWidth = mCurves.at(1).startHalfWidth = 0.02 * (1 + gAmplitude * t);
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

    static constexpr GLsizei gCurveSubdivisions{75};

    math::Box<GLfloat> mViewVolume;
    math::AffineMatrix<3, GLfloat> mWindowToView;
    Curving mCurving;
    std::vector<Curving::Instance> mCurves;

    bool mHandleClicked{false};
};


} // namespace ad
