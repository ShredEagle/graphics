#include "ImguiUi.h"

#include <GLFW/glfw3.h>

#include <cassert>
#include <imgui_backends/imgui_impl_glfw.h>
#include <imgui_backends/imgui_impl_opengl3.h>
#include <mutex>

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
        
        // on first call NewFrame create all the shader 
        // and font texture so it NEEDS to be called
        // once before the first backend render
        // Since we want to avoid all call in the render thread
        // We call this once in the ImguiUi constructor
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

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

void ImguiUi::renderBackend()
{
    std::lock_guard lock{mFrameMutex};
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void ImguiUi::newFrame()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}


void ImguiUi::render()
{
    ImGui::Render();
}


bool ImguiUi::isCapturingKeyboard()
{
    return mIo.WantCaptureKeyboard;
}


bool ImguiUi::isCapturingMouse()
{
    return mIo.WantCaptureMouse;
}


} // namespace imguiui
} // namespace ad
