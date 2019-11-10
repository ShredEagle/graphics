#pragma once

#include "Engine.h"

#include <renderer/Error.h>

#include <handy/Guard.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>

namespace ad
{


struct Application
{
    static void error_callback(int error, const char* description)
    {
        std::cerr << "Application encountered GLFW error: "
                  << description
                  << std::endl;
    }

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    static void windowsSize_callback(GLFWwindow * window, int width, int height)
    {
        ad::Engine * engine = static_cast<ad::Engine *>(glfwGetWindowUserPointer(window));
        engine->callbackWindowSize(width, height);
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
                                                int aGLVersionMajor, int aGLVersionMinor)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, aGLVersionMajor);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, aGLVersionMinor);

        // macOS requirement (as well as not going above to OpenGL 4.1)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Only show the window after its size callback is set
        // so we cannot miss notifications
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

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

    Application(const std::string aName,
                int aWidth, int aHeight,
                int aGLVersionMajor=4, int aGLVersionMinor=1) :
        mGlfwInitialization(initializeGlfw()),
        mWindow(initializeWindow(aName, aWidth, aHeight, aGLVersionMajor, aGLVersionMinor))
    {
        glfwMakeContextCurrent(mWindow);
        gladLoadGL();

        mEngine = std::make_shared<Engine>();
        glfwSetWindowUserPointer(mWindow, mEngine.get());
        // Explicitly call it, because it is used to complete the engine setup
        {
            // Get the size, because the hints might not be satisfied
            // (yet not invoking the size callback)
            int width, height;
            glfwGetWindowSize(mWindow, &width, &height);
            windowsSize_callback(mWindow, width, height);
        }

        glfwSetKeyCallback(mWindow, key_callback);
        glfwSetWindowSizeCallback(mWindow, windowsSize_callback);

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

    bool nextFrame()
    {
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
        return ! glfwWindowShouldClose(mWindow);
    }

    Guard mGlfwInitialization;
    ResourceGuard<GLFWwindow*> mWindow;
    std::shared_ptr<Engine> mEngine;
};

} // namespace ad
