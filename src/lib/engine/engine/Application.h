#pragma once

#include "Engine.h"

#include <renderer/Error.h>
#include <renderer/GL_Loader.h>

#include <handy/Guard.h>

#include <GLFW/glfw3.h>

#include <memory>

namespace ad
{


class Application
{
public:
    enum Flags
    {
        None = 0,
        Window_Keep_Ratio = (1 << 1),
    };

    using WindowHints = std::initializer_list<std::pair</*GLFW int*/int, /*value*/int>>;

    Application(const std::string aName,
                int aWidth, int aHeight,
                Flags aFlags = None,
                int aGLVersionMajor=4, int aGLVersionMinor=1,
                WindowHints aCustomWindowHints = {}) :
        mGlfwInitialization(initializeGlfw()),
        mWindow(initializeWindow(aName, aWidth, aHeight, aGLVersionMajor, aGLVersionMinor, aCustomWindowHints))
    {
        if (aFlags & Window_Keep_Ratio)
        {
            glfwSetWindowAspectRatio(mWindow, aWidth, aHeight);
        }

        glfwMakeContextCurrent(mWindow);
        gladLoadGL();

        mEngine = std::make_shared<Engine>();
        glfwSetWindowUserPointer(mWindow, mEngine.get());
        // Explicitly call size callbacks, they are used to complete the engine setup
        {
            // Get the size, because the hints might not be satisfied
            // (yet not invoking the size callback)
            int width, height;
            glfwGetWindowSize(mWindow, &width, &height);
            windowsSize_callback(mWindow, width, height);

            glfwGetFramebufferSize(mWindow, &width, &height);
            framebufferSize_callback(mWindow, width, height);
        }

        glfwSetWindowSizeCallback(mWindow, windowsSize_callback);
        glfwSetFramebufferSizeCallback(mWindow, framebufferSize_callback);

        using namespace std::placeholders;
        mEngine->registerKeyCallback(std::bind(&Application::default_key_callback,
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
            ad::enableDebugOutput();
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

    std::shared_ptr<Engine> getEngine()
    {
        return mEngine;
    }

    std::shared_ptr<const Engine> getEngine() const
    {
        return mEngine;
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

    bool loadWindowIntoImGui(std::function<bool(GLFWwindow * window, bool install_callbacks)> ImGui_install)
    {
        return ImGui_install(mWindow, true);
    }

private:
    static void error_callback(int error, const char* description)
    {
        std::cerr << "Application encountered GLFW error: "
                  << description
                  << std::endl;
    }

    static void default_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    static void forward_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        ad::Engine * engine = static_cast<ad::Engine *>(glfwGetWindowUserPointer(window));
        engine->callbackKeyboard(key, scancode, action, mods);
    }

    static void forward_mousebutton_callback(GLFWwindow* window, int button, int action, int mods)
    {
        ad::Engine * engine = static_cast<ad::Engine *>(glfwGetWindowUserPointer(window));

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        engine->callbackMouseButton(button, action, mods, xpos, ypos);
    }

    static void forward_cursorposition_callback(GLFWwindow* window, double xpos, double ypos)
    {
        ad::Engine * engine = static_cast<ad::Engine *>(glfwGetWindowUserPointer(window));

        engine->callbackCursorPosition(xpos, ypos);
    }

    static void windowsSize_callback(GLFWwindow * window, int width, int height)
    {
        ad::Engine * engine = static_cast<ad::Engine *>(glfwGetWindowUserPointer(window));
        engine->callbackWindowSize(width, height);
    }

    static void framebufferSize_callback(GLFWwindow * window, int width, int height)
    {
        ad::Engine * engine = static_cast<ad::Engine *>(glfwGetWindowUserPointer(window));
        engine->callbackFramebufferSize(width, height);
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

    ResourceGuard<GLFWwindow*> initializeWindow(const std::string & aName,
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

        auto window = guard(glfwCreateWindow(aWidth,
                                             aHeight,
                                             aName.c_str(),
                                             NULL,
                                             NULL),
                            glfwDestroyWindow);
        if (!window)
        {
            throw std::runtime_error("Unable to initialize window or context");
        }

        return window;
    }

    Guard mGlfwInitialization;
    ResourceGuard<GLFWwindow*> mWindow;
    std::shared_ptr<Engine> mEngine;
};

} // namespace ad
