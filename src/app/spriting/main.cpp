#include <renderer/Error.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>

#include "scene.h"

constexpr int gWindowWidth{1280};
constexpr int gWindowHeight{1024};

constexpr int gGLVersionMajor{4};
constexpr int gGLVersionMinor{3};


static void error_callback(int error, const char* description)
{
    std::cerr << "GLFW error: " << description << std::endl;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void)
{
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        throw std::runtime_error("Unable to initialize glfw");
    }
    Guard glfwInitGuard{glfwTerminate};
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gGLVersionMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gGLVersionMinor);

    auto window = guard(glfwCreateWindow(gWindowWidth,
                                         gWindowHeight,
                                         "2D Demo",
                                         NULL,
                                         NULL),
                                glfwDestroyWindow);
    if (!window)
    {
        throw std::runtime_error("Unable to initialize window or context");
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGL();
    // VSync
    glfwSwapInterval(1);

    ad::enableDebugOutput();
    ad::Scene scene = ad::setupScene();

    while(!glfwWindowShouldClose(window))
    {
        /// \TODO Only do on resize envents
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);    
        glViewport(0, 0, width, height);

        ad::updateScene();
        ad::renderScene();

        glfwSwapBuffers(window);
        glfwPollEvents();
    } 

    std::exit(EXIT_SUCCESS);
}
