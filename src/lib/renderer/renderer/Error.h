#pragma once

#include "GL_Loader.h"

#include <iostream>


namespace ad {
namespace graphics {

// Only starting in 4.3 apparently
inline void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
    std::cerr << "GL CALLBACK: " << ((type == GL_DEBUG_TYPE_ERROR) ? "** GL ERROR **"
                                                                   : "")
              << "type = 0x" << type
              << ", severity = 0x" << severity
              << ", message = " << message
              << std::endl;
}

// During init, can be used to enable debug output
inline void enableDebugOutput(decltype(MessageCallback) aOutputCallback = &MessageCallback)
{
    glEnable              ( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( aOutputCallback, 0 );
}

struct [[nodiscard]] ErrorCheck
{
    ErrorCheck()
    {
        while (glGetError() != GL_NO_ERROR)
        {
            std::cerr << "An error was waiting in the stack";
        }
    }

    ~ErrorCheck()
    {
        while (GLenum status = glGetError())
        {
            std::cerr << "The call generated error: " << std::hex << "0x" <<  status << std::endl;
        }
    }
};

} // namespace graphics
} // namespace ad;
