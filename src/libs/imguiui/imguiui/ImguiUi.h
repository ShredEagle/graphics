#pragma once

#include <imgui.h>

#include <graphics/ApplicationGlfw.h>
#include <memory>
#include <mutex>


namespace ad {
namespace imguiui {


// TODO this should implement RAII (in particular newFrame()/render())
class ImguiUi
{
public:
    /// @brief Capture the ImGuiContext associated to the GLFWWindow.
    /// @note Derives from the "base" user data installed by ApplicationGlfw:
    /// this way, it is still correct to cast to the base type in pre-existing callbacks.
    struct WindowUserData;

    ImguiUi(const graphics::ApplicationGlfw & aApplication);

    ~ImguiUi();

    void renderBackend();

    void newFrame();
    void render();

    bool isCapturingKeyboard() const;
    bool isCapturingMouse() const;

    /// @brief Override DearImgui callbacks with our callbacks, which are ImGui context-aware.
    /// @see https://github.com/ocornut/imgui/issues/7155#issuecomment-1864823995 for rationale.
    void registerGlfwCallbacks(const graphics::ApplicationGlfw & aApplication);

    std::mutex mFrameMutex;

private:
    // Could be pimpled, but if we do not include imgui.h here,
    // the client will need to do it to construct GUIs.
    ImGuiContext * mContext;
    ImGuiIO & mIo;

    // If this instance had userdata, it is assumed to means it called registerGlfwCallbacks
    // In this case, this function will restore the mContext for the duration of the Guard
    // and restore the previous context when the Guard goes out of scope.
    Guard scopeImguiContext() const;

    // The user data associated to the GLFWWindow corresponding to this.
    std::unique_ptr<WindowUserData> mUserData;
};


} // namespace imguiui
} // namespace ad
