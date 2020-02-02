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
    const Size2<int> & getFramebufferSize() const;
    void callbackWindowSize(int width, int height);
    void callbackFramebufferSize(int width, int height);
    Listening listenResize(SizeListener aListener);


    template <class T_keyCallback>
    void registerKeyCallback(std::shared_ptr<T_keyCallback> && mCallbackInstance);
    void callbackKeyboard(int key, int scancode, int action, int mode);

private:
    Size2<int> mWindowSize;
    Size2<int> mFramebufferSize;
    std::vector<SizeListener> mSizeCallbacks;
    std::function<void(int, int, int, int)> mKeyboardCallback;
};


inline const Size2<int> & Engine::getWindowSize() const
{
    return mWindowSize;
}


inline const Size2<int> & Engine::getFramebufferSize() const
{
    return mFramebufferSize;
}

inline Engine::Listening Engine::listenResize(SizeListener aListener)
{
    mSizeCallbacks.push_back(std::move(aListener));
    return {};
}

template <class T_keyCallback>
void Engine::registerKeyCallback(std::shared_ptr<T_keyCallback> && mCallbackInstance)
{
    mKeyboardCallback = [mCallbackInstance](int key, int scancode, int action, int mods)
    {
        (*mCallbackInstance)(key, scancode, action, mods);
    };
}

inline void Engine::callbackKeyboard(int key, int scancode, int action, int mode)
{
    mKeyboardCallback(key, scancode, action, mode);
}

} // namespace ad
