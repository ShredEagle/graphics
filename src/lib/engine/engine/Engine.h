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
    void registerKeyCallback(T_keyCallback && mCallback);
    template <class T_keyCallback>
    void registerKeyCallback(std::shared_ptr<T_keyCallback> aCallback);

    template <class T_mouseButtonCallback>
    void registerMouseButtonCallback(T_mouseButtonCallback && mCallback);

    template <class T_cursorPositionCallback>
    void registerCursorPositionCallback(T_cursorPositionCallback && mCallback);

    /// \brief To be called by the application when the Window is resized
    void callbackWindowSize(int width, int height);
    /// \brief To be called by the application when the Framebuffer is resized
    void callbackFramebufferSize(int width, int height);

    /// \brief To be called by the application when it has keyboard events to provide
    void callbackKeyboard(int key, int scancode, int action, int mods);

    /// \brief To be called by the application when it has mouse button events to provide
    void callbackMouseButton(int button, int action, int mods, double xpos, double ypos);

    /// \brief To be called by the application when it has cursor position events to provide
    void callbackCursorPosition(double xpos, double ypos);

private:
    Size2<int> mWindowSize;
    Size2<int> mFramebufferSize;
    Subject<SizeListener> mFramebufferSizeSubject;
    std::function<void(int, int, int, int)> mKeyboardCallback;
    std::function<void(int, int, int, double, double)> mMouseButtonCallback;
    std::function<void(double, double)> mCursorPositionCallback;
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
void Engine::registerKeyCallback(T_keyCallback && aCallback)
{
    mKeyboardCallback = std::forward<T_keyCallback>(aCallback);
}


template <class T_keyCallback>
void Engine::registerKeyCallback(std::shared_ptr<T_keyCallback> aCallback)
{
    mKeyboardCallback = [aCallback](int key, int scancode, int action, int mods)
    {
        (*aCallback)(key, scancode, action, mods);
    };
}


template <class T_mouseButtonCallback>
void Engine::registerMouseButtonCallback(T_mouseButtonCallback && mCallback)
{
    mMouseButtonCallback = std::forward<T_mouseButtonCallback>(mCallback);
}


template <class T_cursorPositionCallback>
void Engine::registerCursorPositionCallback(T_cursorPositionCallback && mCallback)
{
    mCursorPositionCallback = std::forward<T_cursorPositionCallback>(mCallback);
}


inline void Engine::callbackKeyboard(int key, int scancode, int action, int mods)
{
    mKeyboardCallback(key, scancode, action, mods);
}


inline void Engine::callbackMouseButton(int button, int action, int mods, double xpos, double ypos)
{
    mMouseButtonCallback(button, action, mods, xpos, ypos);
}


inline void Engine::callbackCursorPosition(double xpos, double ypos)
{
    mCursorPositionCallback(xpos, ypos);
}


} // namespace ad
