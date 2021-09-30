#pragma once


#include <engine/Curving.h>
#include <engine/Timer.h>

#include <math/Transformations.h>
#include <math/VectorUtilities.h>


namespace ad
{


math::AffineMatrix<4, GLfloat> getProjection(Size2<int> aRenderResolution,
                                             GLfloat aWindowHeight = 3., GLfloat aNear = 0., GLfloat aDepth = 100.)
{

    math::Size<3, GLfloat> frontSize{
        math::makeSizeFromHeight(aWindowHeight, math::getRatio<GLfloat>(aRenderResolution)),
        aDepth
    };

    return math::trans3d::orthographicProjection(math::Box<GLfloat>{
        math::Position<3, GLfloat>{
            -frontSize.width() / 2.f,
            -frontSize.height() / 2.f,
            aNear},
        frontSize,
    });
}

class Scene
{
public:
    Scene(Size2<int> aRenderResolution) :
        mCurving{ gCurveSubdivisions, getProjection(aRenderResolution) },
        mCurves{
            Curving::Instance{ 
                math::Bezier<4, 3, GLfloat>{
                    math::Position<3, GLfloat>{-0.5f, -0.5f, 0.f},
                    math::Position<3, GLfloat>{-0.3f,  0.5f, 0.f},
                    math::Position<3, GLfloat>{ 0.3f, -0.5f, 0.f},
                    math::Position<3, GLfloat>{ 0.5f,  0.5f, 0.f}
                },
                0.05f,
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
                0.1f
            },
        }
    {}

    void step(const Timer & aTimer)
    {
    }

    void render()
    {
        mCurving.render(mCurves);
    }

private:
    static constexpr GLsizei gCurveSubdivisions{75};

    Curving mCurving;
    std::vector<Curving::Instance> mCurves;
};


} // namespace ad
