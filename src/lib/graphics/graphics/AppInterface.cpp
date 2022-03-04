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
    //glViewport(0, 0, width, height);
    mWindowSize.width() = width;
    mWindowSize.height() = height;

    // NOTE Ad 2022/01/07: It is not possible to listen to the windows resize event for the moment
    // So there are no notifications to send.
}

void AppInterface::callbackFramebufferSize(int width, int height)
{
    ADLOG(gMainLogger, debug)("The framebuffer has been resized to ({}, {}).", width, height);
    glViewport(0, 0, width, height);
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

void AppInterface::setClearColor(math::hdr::Rgb aClearColor)
{
    glClearColor((GLfloat)aClearColor.r(),
                 (GLfloat)aClearColor.g(),
                 (GLfloat)aClearColor.b(),
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
    std::ostringstream oss;
    oss << "(type = 0x" << type
        << ", severity = 0x" << severity
        << ")\n" << message
    ;
    if (type == GL_DEBUG_TYPE_ERROR)
    {
        ADLOG(gOpenglLogger, error)(oss.str());
    }
    else
    {
        ADLOG(gOpenglLogger, info)(oss.str());
    }
}

} // namespace graphics
} // namespace ad
