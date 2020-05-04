#pragma once

#include "commons.h"
#include "Sprite.h"

#include <handy/Observer.h>


namespace ad {

class Engine
{
public:
    using SizeListener = std::function<void(Size2<int>)>;

    Engine();

    void clear();

    const Size2<int> & getWindowSize() const;
    const Size2<int> & getFramebufferSize() const;

    [[nodiscard]] std::shared_ptr<SizeListener> listenFramebufferResize(SizeListener aListener);

    /// \note Takes a callback by value, keep it until replaced or Engine instance is destructed
    template <class T_keyCallback>
    void registerKeyCallback(T_keyCallback mCallback);
    template <class T_keyCallback>
    void registerKeyCallback(std::shared_ptr<T_keyCallback> aCallback);

    /// \brief To be called by the application when the Window is resized
    void callbackWindowSize(int width, int height);
    /// \brief To be called by the application when the Framebuffer is resized
    void callbackFramebufferSize(int width, int height);

    /// \brief To be called by the application when it has keyboard events to provide
    void callbackKeyboard(int key, int scancode, int action, int mode);

private:
    Size2<int> mWindowSize;
    Size2<int> mFramebufferSize;
    Subject<SizeListener> mFramebufferSizeSubject;
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


inline std::shared_ptr<Engine::SizeListener> Engine::listenFramebufferResize(SizeListener aListener)
{
    auto result = std::make_shared<SizeListener>(std::move(aListener));
    mFramebufferSizeSubject.mObservers.emplace_back(result);
    return result;
}


template <class T_keyCallback>
void Engine::registerKeyCallback(T_keyCallback aCallback)
{
    mKeyboardCallback = [aCallback](int key, int scancode, int action, int mods)
    {
        aCallback(key, scancode, action, mods);
    };
}


template <class T_keyCallback>
void Engine::registerKeyCallback(std::shared_ptr<T_keyCallback> aCallback)
{
    mKeyboardCallback = [aCallback](int key, int scancode, int action, int mods)
    {
        (*aCallback)(key, scancode, action, mods);
    };
}


inline void Engine::callbackKeyboard(int key, int scancode, int action, int mode)
{
    mKeyboardCallback(key, scancode, action, mode);
}

} // namespace ad
