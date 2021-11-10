#include "Scene.h"

#include <graphics/ApplicationGlfw.h>
#include <graphics/AppInterface.h>
#include <graphics/Timer.h>

#include <resource/PathProvider.h>

#include <future>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#endif

using namespace ad;
using namespace ad::graphics;


#ifdef _WIN32

std::string windowsWcsToUtf8(const std::wstring & aWcs)
{
    int outputSize = 
        WideCharToMultiByte(CP_UTF8, 0, aWcs.data(), aWcs.length(),
                            nullptr, 0, NULL, NULL);
    std::string result(outputSize, '\0');
    int mbLength =
        WideCharToMultiByte(CP_UTF8, 0, aWcs.data(), aWcs.length(),
                            result.data(), result.size(), NULL, NULL);
    if (mbLength == 0)
    {
        throw std::logic_error{"wcs to utf8 error."};
    }
    return result;
}

std::string windowsMcbsToUtf8(const std::string & aMbcs)
{
    int wcLength = MultiByteToWideChar(CP_ACP, 0, aMbcs.c_str(), aMbcs.size(), nullptr, 0);
    std::wstring buffer(wcLength, '\0');
    wcLength = MultiByteToWideChar(CP_ACP, 0, aMbcs.c_str(), aMbcs.size(), buffer.data(), buffer.size());
    if (wcLength == 0)
    {
        throw std::logic_error{"mbcs to wcs error."};
    }
    return windowsWcsToUtf8(buffer);
};

#endif


std::string readLine()
{
    std::cout << "Your message: " << std::flush;

#ifdef _WIN32
    static constexpr std::size_t gBufferLength = 128;
    DWORD numRead = 0;
    wchar_t buffer[gBufferLength];
    if (ReadConsoleW(GetStdHandle(STD_INPUT_HANDLE), buffer, gBufferLength, &numRead, NULL) == 0)
    {
        throw std::logic_error{"Error reading console."};
    }
    std::wstring message;
    message.append(buffer, numRead);
    // Erase newline from the input...
    message.erase(std::remove(message.begin(), message.end(), '\r'), message.end());
    message.erase(std::remove(message.begin(), message.end(), '\n'), message.end());

    // The approach below does not work on Windows, would be too convenient.
    // see: https://stackoverflow.com/questions/51196029/displaying-a-wchar-c-and-its-code-point-from-stdwcin#comment89384210_51196029
    //std::wstring message;
    //std::getline(std::wcin, message);

    return windowsWcsToUtf8(message);
#elif
    std::string message;
    std::getline(std::cin, message);
    return message;
#endif
}


int main(int argc, const char * argv[])
{
    try
    {
        // I suspect this is the default anyway
        //SetConsoleCP(CP_ACP);
        ApplicationGlfw application("Texting", 1600, 1000,
                                    ApplicationGlfw::Flags::None,
                                    4, 1,
                                    { {GLFW_SAMPLES, 8} });

        Timer timer{glfwGetTime(), 0.};
        Scene scene{resource::pathFor("fonts/dejavu-fonts-ttf-2.37/DejaVuSans.ttf"),
                    application.getAppInterface() };

        std::future line = std::async(&readLine);
        while(application.nextFrame())
        {
            if (line.wait_for(std::chrono::seconds{0}) == std::future_status::ready)
            {
                scene.setMessage(line.get());
                line = std::async(&readLine);
            }
            timer.mark(glfwGetTime());
            scene.step(timer);
            application.getAppInterface()->clear();
            scene.render();
        }
    }
    catch(const std::exception & e)
    {
        std::cerr << "Exception:\n"
                  << e.what()
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::exit(EXIT_SUCCESS);
}
