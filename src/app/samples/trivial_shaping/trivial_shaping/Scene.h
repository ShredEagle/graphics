#pragma once


#include <engine/TrivialShaping.h>
#include <engine/Timer.h>


namespace ad
{


class Scene
{
public:
    Scene(Size2<int> aRenderResolution) :
        mTrivialShaping{std::move(aRenderResolution)}
    {}

    void step(const Timer & aTimer)
    {
        mTrivialShaping.clearShapes();

        mTrivialShaping.addRectangle({{{10.f, 20.f}, {100.f, 50.f}}, Color{255, 255, 255}});
        mTrivialShaping.addRectangle({{{100.f, 150.f}, {10.f, 90.f}}, Color{0, 255, 255}});

        if (static_cast<int>(aTimer.time()) % 2 == 0)
        {
            mTrivialShaping.addRectangle({{{200.f, 100.f}, {180.f, 120.f}}, Color{255, 0, 255}});
        }
    }

    void render()
    {
        mTrivialShaping.render();
    }

private:
    TrivialShaping mTrivialShaping;
};


} // namespace ad
