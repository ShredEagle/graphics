#include "Render.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <functional>
#include <iostream>
#include <stdexcept>

struct [[nodiscard]] GlfwContext
{
    GlfwContext()
    {
        glfwSetErrorCallback(error_callback);

        if (!glfwInit())
        {
            throw std::runtime_error("Unable to initialize glfw");
        }
    }

    ~GlfwContext()
    {
        glfwTerminate();
    }

private:
    static void error_callback(int error, const char* description)
    {
        std::cerr << "GLFW error: " << description << std::endl;
    }
};

template <class T>
class ResourceGuard
{
public:
    typedef std::function<void(T &)> release_fun;
    
    ResourceGuard(T aResource, release_fun aReleaser):
        mResource{std::move(aResource)},
        mReleaser{std::move(aReleaser)}
    {}

    ~ResourceGuard()
    {
        try
        {
            mReleaser(mResource);
        }
        catch(...)
        {
            std::cerr << "Catastrophic failure: resource release threw an exception" 
                      << std::endl;
        }
    } 

    operator T& ()
    {
        return mResource;
    }

private:
    T mResource;
    release_fun mReleaser;
};

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

template <class T>
ResourceGuard<T> guardResource(T aResource, typename ResourceGuard<T>::release_fun aReleaser)
{
    return ResourceGuard<T>(std::move(aResource), std::move(aReleaser));
}

constexpr int gWindowWidth{1280};
constexpr int gWindowHeight{1024};

constexpr int gGLVersionMajor{4};
constexpr int gGLVersionMinor{2};

int main(void)
{
    GlfwContext context;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gGLVersionMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gGLVersionMinor);

    auto window = guardResource(glfwCreateWindow(gWindowWidth,
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

    VertexSpecification specification = initializeGeometry();
    Program program = initializeProgram();

    while(!glfwWindowShouldClose(window))
    {
        // Only do on resize envents
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);    
        glViewport(0, 0, width, height);

        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    } 

    std::exit(EXIT_SUCCESS);
}

