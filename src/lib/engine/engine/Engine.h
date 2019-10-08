#pragma once

#include "commons.h"
#include "Sprite.h"


namespace ad {

class Engine
{
    typedef std::function<void(Size2<int>)> SizeListener;
    /// \todo Implement an RAII structure that removes the listening on destruction
    struct Listening{};

public:
    Engine();

    void clear();

    const Size2<int> & getWindowSize() const;
    void callbackWindowSize(int width, int height);
    Listening listenResize(SizeListener aListener);

private:
    Size2<int> mWindowSize;
    std::vector<SizeListener> mSizeCallbacks;
};


inline const Size2<int> & Engine::getWindowSize() const
{
    return mWindowSize;
}

inline Engine::Listening Engine::listenResize(SizeListener aListener)
{
    mSizeCallbacks.push_back(std::move(aListener));
    return {};
}

} // namespace ad
