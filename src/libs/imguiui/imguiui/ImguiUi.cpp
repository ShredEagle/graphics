#include "ImguiUi.h"

#include <GLFW/glfw3.h>

#include <cassert>
#include <imgui_backends/imgui_impl_glfw.h>
#include <imgui_backends/imgui_impl_opengl3.h>
#include <mutex>


namespace ad {
namespace imguiui {


namespace {

    ImGuiContext * initializeContext()
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGuiContext * context = ImGui::CreateContext();
        // CreateContext only set the current context to the new one if none were present before
        // but we need it set for the following setup
        ImGui::SetCurrentContext(context);
        return context;
    }

    ImGuiIO & initializeImgui(GLFWwindow * aWindow)
    {
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


    ImguiUi::WindowUserData * retrieveUserData(GLFWwindow* window)
    {
        return static_cast<ImguiUi::WindowUserData *>(glfwGetWindowUserPointer(window));
    }


    // Restore the ImGuiContext associated to the GLFWWindow,
    // then forward to the ImGui callback.
    template <auto F_imguiCallback, typename... VA_args>
    void implCallback(GLFWwindow* window, VA_args... aArgs)
    {
        auto userData = retrieveUserData(window);
        // Keeping track of previous context should not be necessary,
        // but let's be good citizens.
        auto previousContext = ImGui::GetCurrentContext();

        ImGui::SetCurrentContext(userData->mContext);
        F_imguiCallback(window, std::forward<VA_args>(aArgs)...);
        ImGui::SetCurrentContext(previousContext);
    }

} // anonymous namespace


ImguiUi::ImguiUi(const graphics::ApplicationGlfw & aApplication) :
    mContext{initializeContext()},
    mIo{initializeImgui(aApplication.getGlfwWindow())}
{
    // #igwp82 Setting the current context to NULL does not work, because
    // ImGui installs a customm window_procedure on _WIN32: ImGui_ImplGlfw_WndProc()
    // this procedures retrieve the ImGui context, which would be null at most points...
    // TODO Ad 2024/08/08: This means that the wrong context is sometimes retrieves when several windows are in flight.
    // We might handle that by overriding with a custom window_proc and user data.
    //ImGui::SetCurrentContext(nullptr);
}


ImguiUi::~ImguiUi()
{
    ImGui::SetCurrentContext(mContext);
    terminateImgui();
}


void ImguiUi::registerGlfwCallbacks(const graphics::ApplicationGlfw & aApplication)
{
    // see https://github.com/ocornut/imgui/issues/7155#issuecomment-1864823995 
    // We associate the ImGui context with each window, this way we can restore the
    // correct context before calling the ImGui callbacks.
    //
    // Note: contrary to the recommandation, we keep install_callback=true.
    // This way, ImGui still captures the previous callback and call them in addition to its logic
    // (i.e. those installed by ApplicationGlfw that forward to AppInterface)

    // Instantiate the augumented WindowUserData: copy the existing user data
    // and add the ImGuiContext pointer.
    GLFWwindow * window = aApplication.getGlfwWindow();
    graphics::ApplicationGlfw::WindowUserData * previousData =
        static_cast<graphics::ApplicationGlfw::WindowUserData *>(glfwGetWindowUserPointer(window));
    mUserData = std::make_unique<WindowUserData>(WindowUserData{*previousData});
    mUserData->mContext = mContext;
    glfwSetWindowUserPointer(window, mUserData.get());

    // Register our custom function for the 7 required glfw callbacks.
    glfwSetWindowFocusCallback(window,  implCallback<ImGui_ImplGlfw_WindowFocusCallback, int>);
    glfwSetCursorEnterCallback(window,  implCallback<ImGui_ImplGlfw_CursorEnterCallback, int>);
    glfwSetCursorPosCallback(window,    implCallback<ImGui_ImplGlfw_CursorPosCallback, double, double>);
    glfwSetMouseButtonCallback(window,  implCallback<ImGui_ImplGlfw_MouseButtonCallback, int, int, int>);
    glfwSetScrollCallback(window,       implCallback<ImGui_ImplGlfw_ScrollCallback, double, double>);
    glfwSetKeyCallback(window,          implCallback<ImGui_ImplGlfw_KeyCallback, int, int, int, int>);
    glfwSetCharCallback(window,         implCallback<ImGui_ImplGlfw_CharCallback, unsigned int>);
}


void ImguiUi::renderBackend()
{
    // TODO Ad 2024/08/08: we should verify that we are locking the right section.
    // (Naively, I would suppose we had to lock starting from ImGui::Render())
    std::lock_guard lock{mFrameMutex};
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Does not work because of the ImGui custom window_procedure, see #igwp82
    //ImGui::SetCurrentContext(nullptr);
}


void ImguiUi::newFrame()
{
    ImGui::SetCurrentContext(mContext);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}


void ImguiUi::render()
{
    ImGui::Render();
}


bool ImguiUi::isCapturingKeyboard() const
{
    return mIo.WantCaptureKeyboard;
}


bool ImguiUi::isCapturingMouse() const
{
    return mIo.WantCaptureMouse;
}


} // namespace imguiui
} // namespace ad
