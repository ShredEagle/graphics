#pragma once

#include "AppInterface.h"

#include <renderer/Error.h>
#include <renderer/GL_Loader.h>

#include <handy/Bitmask.h>
#include <handy/Guard.h>

#include <GLFW/glfw3.h>

#include <memory>


namespace ad {
namespace graphics {


enum class ApplicationFlag
{
    None = 0,
    Window_Keep_Ratio = (1 << 1),
    Fullscreen = (1 << 2),
    HideCursor = (1 << 3),
};


} // namespace graphics


template <>
struct is_bitmask<graphics::ApplicationFlag> : public std::true_type
{};


namespace graphics {

class ApplicationGlfw
{
    inline static constexpr int gGLVersionMajor = 4;
    inline static constexpr int gGLVersionMinor= 1;

public:
    using WindowHints = std::initializer_list<std::pair</*GLFW int*/int, /*value*/int>>;

    ApplicationGlfw(const std::string & aName,
                    int aWidth, int aHeight,
                    ApplicationFlag aFlags = ApplicationFlag::None,
                    int aGLVersionMajor=gGLVersionMajor, int aGLVersionMinor=gGLVersionMinor,
                    WindowHints aCustomWindowHints = {}) :
        ApplicationGlfw{NULL/* no context sharing*/,
                        aName, 
                        aWidth, aHeight,
                        aFlags,
                        aGLVersionMajor, aGLVersionMinor,
                        aCustomWindowHints}
    {}

    /// @brief Overload taking a math::Size instead of distinct parameters.
    ApplicationGlfw(const std::string & aName,
                    math::Size<2, int> aSize,
                    ApplicationFlag aFlags = ApplicationFlag::None,
                    int aGLVersionMajor=gGLVersionMajor, int aGLVersionMinor=gGLVersionMinor,
                    WindowHints aCustomWindowHints = {}) :
        ApplicationGlfw{aName, aSize.width(), aSize.height(), aFlags, aGLVersionMajor, aGLVersionMinor, aCustomWindowHints}
    {}

    
    /// @brief Shares the OpenGL context from `aSharedContext`.
    /// Will specify the same OpenGL version than `aSharedContext`.
    ApplicationGlfw(ApplicationGlfw & aSharedContext,
                    const std::string & aName,
                    int aWidth, int aHeight,
                    ApplicationFlag aFlags = ApplicationFlag::None,
                    WindowHints aCustomWindowHints = {}) :
        ApplicationGlfw{aSharedContext.mWindow, /* Share the context with another window */
                        aName, 
                        aWidth, aHeight,
                        aFlags,
                        glfwGetWindowAttrib(aSharedContext.mWindow, GLFW_CONTEXT_VERSION_MAJOR),
                        glfwGetWindowAttrib(aSharedContext.mWindow, GLFW_CONTEXT_VERSION_MINOR),
                        aCustomWindowHints}
    {}


    /// \brief Must be called from the context-active thread before making context current on another thread.
    void removeCurrentContext()
    {
        // Make the OpenGL context non-current
        // see: https://www.glfw.org/docs/latest/context.html#context_current
        glfwMakeContextCurrent(nullptr);
    }

    /// \brief Make the context current on the calling thread.
    void makeContextCurrent()
    {
        glfwMakeContextCurrent(mWindow);
    }

    void show()
    { 
        glfwShowWindow(mWindow);
        getAppInterface()->callbackWindowVisibility(true);
    }

    void hide()
    { 
        glfwHideWindow(mWindow);
        getAppInterface()->callbackWindowVisibility(false);
    }

    bool isVisible() const
    {
        return glfwGetWindowAttrib(mWindow, GLFW_VISIBLE);
    }

    bool shouldClose() const
    {
        return glfwWindowShouldClose(mWindow);
    }

    bool handleEvents()
    {
        glfwPollEvents();
        return !shouldClose();
    }

    void swapBuffers()
    {
        glfwSwapBuffers(mWindow);
    }

    /// \brief Swap buffers then handle events
    bool nextFrame()
    {
        swapBuffers();
        return handleEvents();
    }

    // TODO should return a plain reference, the app interface will not be valide anyway if this is destroyed.
    std::shared_ptr<AppInterface> getAppInterface()
    {
        return mAppInterface;
    }

    std::shared_ptr<const AppInterface> getAppInterface() const
    {
        return mAppInterface;
    }

    void markWindowShouldClose(int aValue = GLFW_TRUE) const
    {
        glfwSetWindowShouldClose(mWindow, aValue);
    }

    void setInputMode(int mode, int value) const
    {
        glfwSetInputMode(mWindow, mode, value);
    }

    int getKey(int keyCode) const
    {
        return glfwGetKey(mWindow, keyCode);
    }

    void getMousePos(double & aXpos, double & aYpos) const
    {
        double xpos, ypos;
        glfwGetCursorPos(mWindow, &xpos, &ypos);
        aXpos = xpos;
        aYpos = ypos;
    }

    void setWindowTitle(const std::string & aTitle) 
    {
        glfwSetWindowTitle(mWindow, aTitle.c_str());
    }

    /// \important This is breaking the encapsulation for cases where third parties such as dear-ImGui
    /// need access.
    /// Please do not use it for "normal" application management, which should be properly wrapped once 
    /// the need arises.
    GLFWwindow * getGlfwWindow() const
    { return mWindow; }


private:

    ApplicationGlfw(GLFWwindow * aSharedContext,
                    const std::string & aName,
                    int aWidth, int aHeight,
                    ApplicationFlag aFlags,
                    int aGLVersionMajor, int aGLVersionMinor,
                    WindowHints aCustomWindowHints) :
        mGlfwInitialization{},
        mWindow{initializeWindow(aName, 
                                 aFlags,
                                 aWidth, aHeight, 
                                 aGLVersionMajor, aGLVersionMinor,
                                 aCustomWindowHints,
                                 aSharedContext)}
    {
        glfwMakeContextCurrent(mWindow);
        gladLoadGL();

        mAppInterface = std::make_shared<AppInterface>(AppInterface::WindowCallbacks{
            .mClose = [this](){markWindowShouldClose(GLFW_TRUE);},
            .mShow = [this](){show();},
            .mHide = [this](){hide();},
        });
            
        glfwSetWindowUserPointer(mWindow, mAppInterface.get());
        // Explicitly call size callbacks, they are used to complete the appInterface setup
        {
            // Get the size, because the hints might not be satisfied
            // (yet not invoking the size callback)
            int width, height;
            glfwGetWindowSize(mWindow, &width, &height);
            windowSize_callback(mWindow, width, height);

            glfwGetFramebufferSize(mWindow, &width, &height);
            framebufferSize_callback(mWindow, width, height);
        }

        glfwSetWindowIconifyCallback(mWindow, windowMinimize_callback);
        glfwSetWindowSizeCallback(mWindow, windowSize_callback);
        glfwSetFramebufferSizeCallback(mWindow, framebufferSize_callback);

        // Set all callbacks to forward to AppInterface, in turn forwarding to user-specified callbacks.
        glfwSetKeyCallback(mWindow, forward_key_callback);
        glfwSetMouseButtonCallback(mWindow, forward_mousebutton_callback);
        glfwSetCursorPosCallback(mWindow, forward_cursorposition_callback);
        glfwSetScrollCallback(mWindow, forward_scroll_callback);

        // Register a default keyboard callback on the AppInterface, closing the window on Esc.
        using namespace std::placeholders;
        mAppInterface->registerKeyCallback(std::bind(&ApplicationGlfw::default_key_callback,
                                                     static_cast<GLFWwindow*>(this->mWindow),
                                                     _1, _2, _3, _4));

        show();

        // TODO Ad 2023/08/10: Control V-sync via a flag
        // VSync
        glfwSwapInterval(1);

        if (!GLAD_GL_KHR_debug)
        {
            std::cerr << "Debug output is not available."
                      << " Please run on a decent platform for debugging."
                      << std::endl;
        }
        else
        {
            enableDebugOutput(&AppInterface::OpenGLMessageLogging);
        }
    }
    static void error_callback(int error, const char* description)
    {
        std::cerr << "Application encountered GLFW error (code " << error << "): "
                  << description
                  << std::endl;
    }

    static void default_key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    static void forward_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        AppInterface * appInterface = static_cast<AppInterface *>(glfwGetWindowUserPointer(window));
        appInterface->callbackKeyboard(key, scancode, action, mods);
    }

    static void forward_mousebutton_callback(GLFWwindow* window, int button, int action, int mods)
    {
        AppInterface * appInterface = static_cast<AppInterface *>(glfwGetWindowUserPointer(window));

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        appInterface->callbackMouseButton(button, action, mods, xpos, ypos);
    }

    static void forward_cursorposition_callback(GLFWwindow* window, double xpos, double ypos)
    {
        AppInterface * appInterface = static_cast<AppInterface *>(glfwGetWindowUserPointer(window));
        appInterface->callbackCursorPosition(xpos, ypos);
    }
    
    static void forward_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        AppInterface * appInterface = static_cast<AppInterface *>(glfwGetWindowUserPointer(window));
        appInterface->callbackScroll(xoffset, yoffset);
    }

    static void windowMinimize_callback(GLFWwindow * window, int iconified)
    {
        AppInterface * appInterface = static_cast<AppInterface *>(glfwGetWindowUserPointer(window));
        appInterface->callbackWindowMinimize(iconified);
    }

    static void windowSize_callback(GLFWwindow * window, int width, int height)
    {
        AppInterface * appInterface = static_cast<AppInterface *>(glfwGetWindowUserPointer(window));
        appInterface->callbackWindowSize(width, height);
    }

    static void framebufferSize_callback(GLFWwindow * window, int width, int height)
    {
        AppInterface * appInterface = static_cast<AppInterface *>(glfwGetWindowUserPointer(window));
        appInterface->callbackFramebufferSize(width, height);
    }

    static Guard InitializeGlfw()
    {
        glfwSetErrorCallback(error_callback);
        if (!glfwInit())
        {
            throw std::runtime_error("Unable to initialize glfw");
        }
        return Guard{glfwTerminate};
    }

    /// \important Fullscreen will use the primary monitor at its current resolution,
    /// ignoring `aWidth` and `aHeight`.
    ResourceGuard<GLFWwindow*> initializeWindow(const std::string & aName,
                                                ApplicationFlag aFlags,
                                                int aWidth, int aHeight,
                                                int aGLVersionMajor, int aGLVersionMinor,
                                                WindowHints aCustomWindowHints,
                                                GLFWwindow * aShare)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, aGLVersionMajor);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, aGLVersionMinor);

        // macOS requirement (as well as not going above to OpenGL 4.1)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        for (auto hint : aCustomWindowHints)
        {
            glfwWindowHint(hint.first, hint.second);
        }

        // Only show the window after its size callback is set
        // so we cannot miss notifications
        // Done after custom hints, so they cannot override it.
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        auto window = [&]()
        {
            if (test(aFlags, ApplicationFlag::Fullscreen))
            {
                GLFWmonitor * monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode * mode = glfwGetVideoMode(monitor);

                // Avoid changing the video-mode
                // see: https://www.glfw.org/docs/3.3/window_guide.html#window_windowed_full_screen
                // NOTE Ad 31/01/2022: I do not see a difference when those hints are not given, there
                // is still a black screen "flashing" at window creation.
                glfwWindowHint(GLFW_RED_BITS,   mode->redBits);
                glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
                glfwWindowHint(GLFW_BLUE_BITS,  mode->blueBits);
                glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

                return guard(glfwCreateWindow(mode->width,
                                              mode->height,
                                              aName.c_str(),
                                              monitor,
                                              aShare),
                             glfwDestroyWindow);
            }
            else
            {
                return guard(glfwCreateWindow(aWidth,
                                              aHeight,
                                              aName.c_str(),
                                              NULL,
                                              aShare),
                             glfwDestroyWindow);
            }
        }();

        if (!window)
        {
            throw std::runtime_error("Unable to initialize window or context");
        }
        else
        {
            if (test(aFlags, ApplicationFlag::Window_Keep_Ratio))
            {
                glfwSetWindowAspectRatio(mWindow, aWidth, aHeight);
            }

            if (test(aFlags, ApplicationFlag::HideCursor))
            {
                glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            }

            return window;
        }
    }

    // This structure initializes a static variable while initializing Glfw, and terminate Glfw on variable destruction.
    // This initialize Glfw only once (not a requirement), and more importantly destruct it only once after main() returns.
    // At this point no more GLFWwindow should still exist.
    struct InitGlfw
    {
        InitGlfw()
        { 
            static Guard mGuardedInit{InitializeGlfw()};
        }
    } mGlfwInitialization;
    ResourceGuard<GLFWwindow*> mWindow;
    std::shared_ptr<AppInterface> mAppInterface;
};


enum class EscKeyBehaviour
{
    Ignore,
    Close,
};


struct NullInhibiter
{
    NullInhibiter() = default;

    bool isCapturingKeyboard() const
    { return false; }

    bool isCapturingMouse() const
    { return false; }

    // Cannot be inline, I suppose because the type declaration is not complete at this point.
    static const NullInhibiter gInstance;
};


// TODO Ad 2024/05/28:
// only register the subset actually provided by T_callbackProvider (so it does not need to implement them all)
/// \brief Register all callbacks at once, that should be available as member functions of `aProvider`.
template <class T_callbackProvider, class T_inhibiter>
void registerGlfwCallbacks(graphics::AppInterface & aAppInterface,
                           T_callbackProvider & aProvider,
                           EscKeyBehaviour aEscBehaviour,
                           const T_inhibiter * aInhibiter = &NullInhibiter::gInstance)
{
    using namespace std::placeholders;

    aAppInterface.registerMouseButtonCallback(
        [aInhibiter, &aProvider](int button, int action, int mods, double xpos, double ypos)
        {
            if(!aInhibiter->isCapturingMouse())
            {
                aProvider.callbackMouseButton(button, action, mods, xpos, ypos);
            }
        });
    aAppInterface.registerCursorPositionCallback(
        [aInhibiter, &aProvider](double xpos, double ypos)
        {
            if(!aInhibiter->isCapturingMouse())
            {
                aProvider.callbackCursorPosition(xpos, ypos);
            }
        });
    aAppInterface.registerScrollCallback(
        [aInhibiter, &aProvider](double xoffset, double yoffset)
        {
            if(!aInhibiter->isCapturingMouse())
            {
                aProvider.callbackScroll(xoffset, yoffset);
            }
        });

    switch(aEscBehaviour)
    {
        case EscKeyBehaviour::Ignore:
            aAppInterface.registerKeyCallback([aInhibiter, &aProvider](int key, int scancode, int action, int mods)
            {
                if(!aInhibiter->isCapturingKeyboard())
                {
                    aProvider.callbackKeyboard(key, scancode, action, mods);
                }
            });
            break;
        case EscKeyBehaviour::Close:
            aAppInterface.registerKeyCallback(
                [&aAppInterface, &aProvider, aInhibiter](int key, int scancode, int action, int mods)
                {
                    if(!aInhibiter->isCapturingKeyboard())
                    {
                        // TODO would be cleaner to factorize that and the ApplicationGlfw::default_key_callback
                        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                        {
                            aAppInterface.requestCloseWindow();
                        }
                        aProvider.callbackKeyboard(key, scancode, action, mods);
                    }
                });
            break;
    }
}

} // namespace graphics
} // namespace ad
