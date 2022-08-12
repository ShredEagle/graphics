#include "ImguiUi.h"

#include <GLFW/glfw3.h>

#include <imgui_backends/imgui_impl_glfw.h>
#include <imgui_backends/imgui_impl_opengl3.h>

namespace {

    ImGuiIO & initializeImgui(GLFWwindow * aWindow)
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(aWindow, true);
        ImGui_ImplOpenGL3_Init();

        return io;
    }


    void terminateImgui()
    {
        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

} // anonymous namespace


namespace ad {
namespace imguiui {


ImguiUi::ImguiUi(const graphics::ApplicationGlfw & aApplication) :
    mIo{initializeImgui(aApplication.getGlfwWindow())}
{}


ImguiUi::~ImguiUi()
{
    terminateImgui();
}


void ImguiUi::newFrame() const
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}


void ImguiUi::render() const
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


bool ImguiUi::isCapturingKeyboard() const
{
    return mIo.WantCaptureKeyboard;
}


bool ImguiUi::isCapturingMouse() const
{
    return mIo.WantCaptureMouse;
}

} // namespace gltfviewer
} // namespace ad
