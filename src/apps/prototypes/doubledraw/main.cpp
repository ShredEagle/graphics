#include <renderer/Error.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>

#include "scene.h"

constexpr int gWindowWidth{1280};
constexpr int gWindowHeight{1024};

constexpr int gGLVersionMajor{4};
constexpr int gGLVersionMinor{1};


static void error_callback(int error, const char* description)
{
    std::cerr << "GLFW error: " << description << std::endl;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void windowsSize_callback(GLFWwindow * window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main(void)
{
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        throw std::runtime_error("Unable to initialize glfw");
    }
    ad::Guard glfwInitGuard{glfwTerminate};

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gGLVersionMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gGLVersionMinor);

    // macOS requirement (as well as not going above to OpenGL 4.1)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto window = ad::guard(glfwCreateWindow(gWindowWidth,
                                             gWindowHeight,
                                             "Double Draw Demo",
                                             NULL,
                                             NULL),
                                             glfwDestroyWindow);
    if (!window)
    {
        throw std::runtime_error("Unable to initialize window or context");
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, windowsSize_callback);

    glfwMakeContextCurrent(window);
    gladLoadGL();
    // VSync
    glfwSwapInterval(1);

    if (!GLAD_GL_KHR_debug)
    {
        std::cerr << "Debug output is not available."
                  << " Please run on a decent platform for debugging.\n";
    }
    else
    {
        ad::graphics::enableDebugOutput();
    }

    ad::graphics::Scene scene = ad::graphics::setupScene();

    while(!glfwWindowShouldClose(window))
    {
        ad::graphics::updateScene(scene, glfwGetTime());
        ad::graphics::renderScene(scene);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    std::exit(EXIT_SUCCESS);
}
