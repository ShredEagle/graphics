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

        mTrivialShaping.addRectangle({{{10, 20}, {100, 50}}, Color{255, 255, 255}});
        mTrivialShaping.addRectangle({{{100, 150}, {10, 90}}, Color{0, 255, 255}});

        if (static_cast<int>(aTimer.time()) % 2 == 0)
        {
            mTrivialShaping.addRectangle({{{200, 100}, {180, 120}}, Color{255, 0, 255}});
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
