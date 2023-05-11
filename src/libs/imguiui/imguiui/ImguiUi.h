#pragma once

#include <imgui.h>

#include <graphics/ApplicationGlfw.h>
#include <mutex>


namespace ad {
namespace imguiui {


// TODO this should implement RAII (in particular newFrame()/render())
class ImguiUi
{
public:
    ImguiUi(const graphics::ApplicationGlfw & aApplication);

    ~ImguiUi();

    void renderBackend();

    void newFrame();
    void render();

    bool isCapturingKeyboard();
    bool isCapturingMouse();

    std::mutex mFrameMutex;

private:
    // Could be pimpled, but if we do not include imgui.h here, the client will need to to it.
    ImGuiIO & mIo;
};


} // namespace imguiui
} // namespace ad
