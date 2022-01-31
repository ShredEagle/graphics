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
};


} // namespace graphics


template <>
struct is_bitmask<graphics::ApplicationFlag> : public std::true_type
{};


namespace graphics {

class ApplicationGlfw
{
public:
    using WindowHints = std::initializer_list<std::pair</*GLFW int*/int, /*value*/int>>;

    ApplicationGlfw(const std::string & aName,
                    math::Size<2, int> aSize,
                    ApplicationFlag aFlags = ApplicationFlag::None,
                    int aGLVersionMajor=4, int aGLVersionMinor=1,
                    WindowHints aCustomWindowHints = {}) :
        ApplicationGlfw{aName, aSize.width(), aSize.height(), aFlags, aGLVersionMajor, aGLVersionMinor, aCustomWindowHints}
    {}

    ApplicationGlfw(const std::string & aName,
                    int aWidth, int aHeight,
                    ApplicationFlag aFlags = ApplicationFlag::None,
                    int aGLVersionMajor=4, int aGLVersionMinor=1,
                    WindowHints aCustomWindowHints = {}) :
        mGlfwInitialization(initializeGlfw()),
        mWindow(initializeWindow(aName, 
                                 test(aFlags, ApplicationFlag::Fullscreen),
                                 aWidth, aHeight, 
                                 aGLVersionMajor, aGLVersionMinor,
                                 aCustomWindowHints))
    {
        if ((aFlags & ApplicationFlag::Window_Keep_Ratio) != ApplicationFlag::None)
        {
            glfwSetWindowAspectRatio(mWindow, aWidth, aHeight);
        }

        glfwMakeContextCurrent(mWindow);
        gladLoadGL();

        mAppInterface = std::make_shared<AppInterface>(
            [this](){glfwSetWindowShouldClose(mWindow, GLFW_TRUE);});
            
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

        using namespace std::placeholders;
        mAppInterface->registerKeyCallback(std::bind(&ApplicationGlfw::default_key_callback,
                                                     static_cast<GLFWwindow*>(this->mWindow),
                                                     _1, _2, _3, _4));
        glfwSetKeyCallback(mWindow, forward_key_callback);

        glfwSetMouseButtonCallback(mWindow, forward_mousebutton_callback);

        glfwSetCursorPosCallback(mWindow, forward_cursorposition_callback);

        glfwShowWindow(mWindow);

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

    bool handleEvents()
    {
        glfwPollEvents();
        return ! glfwWindowShouldClose(mWindow);
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

    std::shared_ptr<AppInterface> getAppInterface()
    {
        return mAppInterface;
    }

    std::shared_ptr<const AppInterface> getAppInterface() const
    {
        return mAppInterface;
    }

    void markWindowShouldClose() const
    {
        glfwSetWindowShouldClose(mWindow, GLFW_TRUE);
    }

    void setInputMode(int mode, int value) const
    {
        glfwSetInputMode(mWindow, mode, value);
    }

    int getKey(int keyCode) const
    {
        return glfwGetKey(mWindow, keyCode);
    }

    void getMousePos(float & aXpos, float & aYpos) const
    {
        double xpos, ypos;
        glfwGetCursorPos(mWindow, &xpos, &ypos);
        aXpos = xpos;
        aYpos = ypos;
    }

    bool loadWindowIntoImGui(std::function<bool(GLFWwindow * window, bool install_callbacks)> ImGui_install)
    {
        return ImGui_install(mWindow, true);
    }

private:
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

    Guard initializeGlfw()
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
                                                bool aFullscreen,
                                                int aWidth, int aHeight,
                                                int aGLVersionMajor, int aGLVersionMinor,
                                                WindowHints aCustomWindowHints)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, aGLVersionMajor);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, aGLVersionMinor);

        // macOS requirement (as well as not going above to OpenGL 4.1)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Only show the window after its size callback is set
        // so we cannot miss notifications
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        for (auto hint : aCustomWindowHints)
        {
            glfwWindowHint(hint.first, hint.second);
        }

        auto window = [&]()
        {
            if (aFullscreen)
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
                                              NULL),
                             glfwDestroyWindow);
            }
            else
            {
                return guard(glfwCreateWindow(aWidth,
                                              aHeight,
                                              aName.c_str(),
                                              NULL,
                                              NULL),
                             glfwDestroyWindow);
            }
        }();

        if (!window)
        {
            throw std::runtime_error("Unable to initialize window or context");
        }

        return window;
    }

    Guard mGlfwInitialization;
    ResourceGuard<GLFWwindow*> mWindow;
    std::shared_ptr<AppInterface> mAppInterface;
};

} // namespace graphics
} // namespace ad
