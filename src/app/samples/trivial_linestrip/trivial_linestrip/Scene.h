#pragma once


#include <graphics/TrivialLineStrip.h>
#include <graphics/Timer.h>


namespace ad {
namespace graphics {


class Scene
{
public:
    Scene(Size2<int> aRenderResolution) :
        mTrivialLineStrip{std::move(aRenderResolution)}
    {}

    void step(const Timer & aTimer)
    {
        mTrivialLineStrip.clearLines();

        mTrivialLineStrip.addLine({
            { { 10.f,  20.f}, math::sdr::gRed },
            { { 90.f, 130.f}, math::sdr::gGreen },
            { { 90.f,  20.f}, math::sdr::gBlue },
        });

        mTrivialLineStrip.addLine({
            { {110.f,  20.f}, math::sdr::gRed },
            { {110.f, 130.f}, math::sdr::gBlue },
            { {190.f, 100.f}, math::sdr::gMagenta },
            { {190.f,  50.f}, math::sdr::gCyan },
            { {130.f,  20.f}, math::sdr::gYellow },
        });

        if (static_cast<int>(aTimer.time()) % 2 == 0)
        {
            mTrivialLineStrip.outlineRectangle({{95.f, 15.f}, {10.f, 10.f}}, Color{255, 0, 255});
        }
    }

    void render()
    {
        mTrivialLineStrip.render();
    }

private:
    TrivialLineStrip mTrivialLineStrip;
};


} // namespace graphics
} // namespace ad
