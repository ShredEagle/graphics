#include "AppInterface.h"

#include "detail/Logging.h"

#include <sstream>


namespace ad {
namespace graphics {


AppInterface::AppInterface(std::function<void()> aCloseAppCallback) :
    mWindowSize{0, 0},
    mFramebufferSize{0, 0},
    mCloseAppCallback{std::move(aCloseAppCallback)}
{
    //
    // General OpenGL setups
    //

    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Frame buffer clear color
    glClearColor(0.1f, 0.2f, 0.3f, 1.f);

    // Initialize logging (as this class should be instantiated in any case)
    detail::initializeLogging();
}


void AppInterface::requestCloseApplication()
{
    mCloseAppCallback();
}


void AppInterface::callbackWindowMinimize(bool aMinimized)
{
    if (aMinimized)
    {
        ADLOG(gMainLogger, info)("The window has be minimized.");
    }
    else
    {
        ADLOG(gMainLogger, info)("The window has be restored.");
    }
    mWindowIsMinimized = aMinimized;
}


void AppInterface::callbackWindowSize(int width, int height)
{
    ADLOG(gMainLogger, debug)("The window has been resized to ({}, {}).", width, height);
    mWindowSize.width() = width;
    mWindowSize.height() = height;

    if (!mWindowIsMinimized)
    {
        mWindowSizeSubject.dispatch(mFramebufferSize);
    }
}


void AppInterface::callbackFramebufferSize(int width, int height)
{
    ADLOG(gMainLogger, debug)("The framebuffer has been resized to ({}, {}).", width, height);
    // Cannot resize the OpenGL viewport here directly:
    // this callback should always be invoked on the thread that created the window
    // but the OpenGL context might have been moved to another thread.
    // Thus, the client is now expected to register a callback invoking `glViewport` if it is needed.
    //glViewport(0, 0, width, height);
    mFramebufferSize.width() = width;
    mFramebufferSize.height() = height;
    if (!mWindowIsMinimized)
    {
        mFramebufferSizeSubject.dispatch(mFramebufferSize);
    }
}


void AppInterface::clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}


void AppInterface::setClearColor(math::hdr::Rgb_f aClearColor)
{
    glClearColor(aClearColor.r(),
                 aClearColor.g(),
                 aClearColor.b(),
                 1.f);
}


void GLAPIENTRY AppInterface::OpenGLMessageLogging(GLenum source,
                                                   GLenum type,
                                                   GLuint id,
                                                   GLenum severity,
                                                   GLsizei length,
                                                   const GLchar* message,
                                                   const void* userParam)
{
    std::string sourceString = [source]() -> std::string
    {
        switch(source)
        {
            case GL_DEBUG_SOURCE_API:
                return "api";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                return "window_system";
            case GL_DEBUG_SOURCE_SHADER_COMPILER:
                return "shader_compiler";
            case GL_DEBUG_SOURCE_THIRD_PARTY:
                return "third_party";
            case GL_DEBUG_SOURCE_APPLICATION:
                return "application";
            case GL_DEBUG_SOURCE_OTHER:
                return "other";
            default:
                return "<UNKNOWN> " + std::to_string(source);
        }
    }();

    std::string typeString = [type]() -> std::string
    {
        switch(type)
        {
            case GL_DEBUG_TYPE_ERROR:
                return "error";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
                return "deprecated";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
                return "undefined";
            case GL_DEBUG_TYPE_PORTABILITY:
                return "portability";
            case GL_DEBUG_TYPE_PERFORMANCE:
                return "performance";
            case GL_DEBUG_TYPE_MARKER:
                return "marker";
            case GL_DEBUG_TYPE_PUSH_GROUP:
                return "push group";
            case GL_DEBUG_TYPE_POP_GROUP:
                return "pop group";
            case GL_DEBUG_TYPE_OTHER:
                return "other";
            default:
                return "<UNKNOWN> " + std::to_string(type);
        }
    }();

    std::string severityString = [severity]() -> std::string
    {
        switch(severity)
        {
            case GL_DEBUG_SEVERITY_HIGH:
                return "high";
            case GL_DEBUG_SEVERITY_MEDIUM:
                return "medium";
            case GL_DEBUG_SEVERITY_LOW:
                return "low";
            case GL_DEBUG_SEVERITY_NOTIFICATION:
                return "notification";
            default:
                return "<UNKNOWN> " + std::to_string(severity);
        }
    }();

    std::ostringstream oss;
    oss << "(source = " << sourceString
        << ", type = " << typeString
        << ", severity = " << severityString
        << ", id = " << id
        << ")\n" << message
    ;

    switch(type)
    {
        case GL_DEBUG_TYPE_ERROR:
        {
            ADLOG(gOpenglLogger, error)(oss.str());
            break;
        }
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        case GL_DEBUG_TYPE_PORTABILITY:
        case GL_DEBUG_TYPE_PERFORMANCE:
        {
            ADLOG(gOpenglLogger, warn)(oss.str());
            break;
        }
        default:
        {
            if(severity == GL_DEBUG_SEVERITY_NOTIFICATION)
            {
                ADLOG(gOpenglLogger, trace)(oss.str());
            }
            else
            {
                ADLOG(gOpenglLogger, debug)(oss.str());
            }
            break;
        }
    }
}


} // namespace graphics
} // namespace ad
