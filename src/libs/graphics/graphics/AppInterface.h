#pragma once

#include "commons.h"
#include "Sprite.h"

#include <handy/Observer.h>

#include <functional>


namespace ad {
namespace graphics {


class AppInterface
{
public:
    using SizeListener = std::function<void(Size2<int>)>;

    AppInterface(std::function<void()> aCloseAppCallback);

    // TODO remove those, these are graphics API level
    static void clear();
    static void setClearColor(math::hdr::Rgb_f aClearColor);

    const Size2<int> & getWindowSize() const;
    const Size2<int> & getFramebufferSize() const;

    [[nodiscard]] std::shared_ptr<SizeListener> listenWindowResize(SizeListener aListener);

    [[nodiscard]] std::shared_ptr<SizeListener> listenFramebufferResize(SizeListener aListener);

    void requestCloseApplication(); 

    /// \note Takes a callback by value, keep it until replaced or AppInterface instance is destructed
    template <class T_keyCallback>
    void registerKeyCallback(T_keyCallback && mCallback);
    template <class T_keyCallback>
    void registerKeyCallback(std::shared_ptr<T_keyCallback> aCallback);

    /// \important Coordinate system origin is TOP-left corner of the window (unlike openGL).
    template <class T_mouseButtonCallback>
    void registerMouseButtonCallback(T_mouseButtonCallback && mCallback);

    template <class T_cursorPositionCallback>
    void registerCursorPositionCallback(T_cursorPositionCallback && mCallback);

    template <class T_scrollCallback>
    void registerScrollCallback(T_scrollCallback && mCallback);

    /// \brief To be called by the application when the Window is minimized (iconified).
    void callbackWindowMinimize(bool aMinimized);

    /// \brief To be called by the application when the Window is resized
    void callbackWindowSize(int width, int height);
    /// \brief To be called by the application when the Framebuffer is resized
    /// \important This method will not resize the OpenGL viewport, it is the responsibility
    /// of the client. Registering a custom listener is a potential approach.
    void callbackFramebufferSize(int width, int height);

    /// \brief To be called by the application when it has keyboard events to provide
    void callbackKeyboard(int key, int scancode, int action, int mods);

    /// \brief To be called by the application when it has mouse button events to provide
    void callbackMouseButton(int button, int action, int mods, double xpos, double ypos);

    /// \brief To be called by the application when it has cursor position events to provide
    void callbackCursorPosition(double xpos, double ypos);

    /// \brief To be called by the application when it has scroll events to provide
    void callbackScroll(double xoffset, double yoffset);

    static void GLAPIENTRY OpenGLMessageLogging(GLenum source,
                                                GLenum type,
                                                GLuint id,
                                                GLenum severity,
                                                GLsizei length,
                                                const GLchar* message,
                                                const void* userParam);

private:
    bool mWindowIsMinimized{false};
    Size2<int> mWindowSize;
    Size2<int> mFramebufferSize;
    Subject<SizeListener> mWindowSizeSubject;
    Subject<SizeListener> mFramebufferSizeSubject;
    std::function<void()> mCloseAppCallback;
    std::function<void(int, int, int, int)> mKeyboardCallback;
    std::function<void(int, int, int, double, double)> mMouseButtonCallback = [](int, int, int, double, double){};
    std::function<void(double, double)> mCursorPositionCallback = [](double, double){};
    std::function<void(double, double)> mScrollCallback = [](double, double){};
};


//
// Implementations
//

inline const Size2<int> & AppInterface::getWindowSize() const
{
    return mWindowSize;
}


inline const Size2<int> & AppInterface::getFramebufferSize() const
{
    return mFramebufferSize;
}


inline std::shared_ptr<AppInterface::SizeListener> AppInterface::listenWindowResize(SizeListener aListener)
{
    auto result = std::make_shared<SizeListener>(std::move(aListener));
    mWindowSizeSubject.mObservers.emplace_back(result);
    return result;
}


inline std::shared_ptr<AppInterface::SizeListener> AppInterface::listenFramebufferResize(SizeListener aListener)
{
    auto result = std::make_shared<SizeListener>(std::move(aListener));
    mFramebufferSizeSubject.mObservers.emplace_back(result);
    return result;
}


template <class T_keyCallback>
void AppInterface::registerKeyCallback(T_keyCallback && aCallback)
{
    mKeyboardCallback = std::forward<T_keyCallback>(aCallback);
}


template <class T_keyCallback>
void AppInterface::registerKeyCallback(std::shared_ptr<T_keyCallback> aCallback)
{
    mKeyboardCallback = [aCallback](int key, int scancode, int action, int mods)
    {
        (*aCallback)(key, scancode, action, mods);
    };
}


template <class T_mouseButtonCallback>
void AppInterface::registerMouseButtonCallback(T_mouseButtonCallback && mCallback)
{
    mMouseButtonCallback = std::forward<T_mouseButtonCallback>(mCallback);
}


template <class T_cursorPositionCallback>
void AppInterface::registerCursorPositionCallback(T_cursorPositionCallback && mCallback)
{
    mCursorPositionCallback = std::forward<T_cursorPositionCallback>(mCallback);
}


template <class T_scrollCallback>
void AppInterface::registerScrollCallback(T_scrollCallback && mCallback)
{
    mScrollCallback = std::forward<T_scrollCallback>(mCallback);
}


inline void AppInterface::callbackKeyboard(int key, int scancode, int action, int mods)
{
    mKeyboardCallback(key, scancode, action, mods);
}


inline void AppInterface::callbackMouseButton(int button, int action, int mods, double xpos, double ypos)
{
    mMouseButtonCallback(button, action, mods, xpos, ypos);
}


inline void AppInterface::callbackCursorPosition(double xpos, double ypos)
{
    mCursorPositionCallback(xpos, ypos);
}


inline void AppInterface::callbackScroll(double xoffset, double yoffset)
{
    mScrollCallback(xoffset, yoffset);
}


} // namespace graphics
} // namespace ad
