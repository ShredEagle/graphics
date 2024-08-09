#include "ImguiUi.h"

#include <GLFW/glfw3.h>

#include <cassert>
#include <imgui_backends/imgui_impl_glfw.h>
#include <imgui_backends/imgui_impl_opengl3.h>
#include <mutex>

// see: 
// https://github.com/ocornut/imgui/blob/6f7b5d0ee2fe9948ab871a530888a6dc5c960700/backends/imgui_impl_glfw.cpp#L86C1-L92C7
#ifdef _WIN32
#undef APIENTRY
#ifndef GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>   // for glfwGetWin32Window()
#endif // _WIN32


namespace ad {
namespace imguiui {


/// @brief Capture the ImGuiContext associated to the GLFWWindow.
/// @note Derives from the "base" user data installed by ApplicationGlfw:
/// this way, it is still correct to cast to the base type in pre-existing callbacks.
struct ImguiUi::WindowUserData : public graphics::ApplicationGlfw::WindowUserData
{
    ImGuiContext * mContext; 
#ifdef  _WIN32
    WNDPROC mPreviousWndProc; // The wndproc that was installed by ImGui
#endif
};


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

    Guard scopeContext(ImGuiContext * aContext)
    {
        Guard guard{[previous = ImGui::GetCurrentContext()]
        {
            ImGui::SetCurrentContext(previous);
        }};
        ImGui::SetCurrentContext(aContext);
        return guard;
    }


#ifdef  _WIN32
    // This window procedure will restore the context (from user data) before forwarding to previous WndProc.
    static LRESULT CALLBACK contextAwareWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        ImguiUi::WindowUserData * userdata = 
            (ImguiUi::WindowUserData *)::GetWindowLongPtrW(hWnd, GWLP_USERDATA);

        //std::cerr << "Calling WndProc for window " << hWnd 
        //        << " with user data " << userdata
        //        << " on thread " << std::this_thread::get_id()
        //        << "\n";

        auto scopedContext = scopeContext(userdata->mContext);

        return ::CallWindowProc(userdata->mPreviousWndProc, hWnd, msg, wParam, lParam);
    }
#endif // _WIN32


} // anonymous namespace


ImguiUi::ImguiUi(const graphics::ApplicationGlfw & aApplication) :
    mContext{initializeContext()},
    mIo{initializeImgui(aApplication.getGlfwWindow())}
{
    // #igwp82 Setting the current context to NULL does not work, because
    // ImGui installs a custom window_procedure on _WIN32: ImGui_ImplGlfw_WndProc()
    // this procedures retrieve the ImGui context, which would be null at most points...
    // Note: We now install a custom context-aware WndProc (restoring the context, then forwarding to ImGui WndProc)
    // yet it is only installed during registerGlfwCallbacks, so still cannot set it to null at this point
    //ImGui::SetCurrentContext(nullptr);
}


ImguiUi::~ImguiUi()
{
    ImGui::SetCurrentContext(mContext);
    // Note: will restore the WndProc saved by ImGui, we do not handle it ourselves
    terminateImgui();
}


Guard ImguiUi::scopeImguiContext() const
{
    if(mUserData != nullptr)
    {
        return scopeContext(mContext);
    }
    else
    {
        return Guard{[](){}};
    }
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

    // Once an UI decide it handles the context itself, we assume it will always restore its context when needed
    ImGui::SetCurrentContext(nullptr);
    // For the rest of this function, activate this context (null context will be restored when leaving the function)
    auto scopedContext = scopeImguiContext();

    // Register our custom function for the 7 required glfw callbacks.
    glfwSetWindowFocusCallback(window,  implCallback<ImGui_ImplGlfw_WindowFocusCallback, int>);
    glfwSetCursorEnterCallback(window,  implCallback<ImGui_ImplGlfw_CursorEnterCallback, int>);
    glfwSetCursorPosCallback(window,    implCallback<ImGui_ImplGlfw_CursorPosCallback, double, double>);
    glfwSetMouseButtonCallback(window,  implCallback<ImGui_ImplGlfw_MouseButtonCallback, int, int, int>);
    glfwSetScrollCallback(window,       implCallback<ImGui_ImplGlfw_ScrollCallback, double, double>);
    glfwSetKeyCallback(window,          implCallback<ImGui_ImplGlfw_KeyCallback, int, int, int, int>);
    glfwSetCharCallback(window,         implCallback<ImGui_ImplGlfw_CharCallback, unsigned int>);

#ifdef _WIN32
        // Partially addresses #igwp82
        // Note: An issue with the current design is that registering glfw callbacks is optional (unneed when there is a single window)
        // so this does not happen at construction.
        // So a secondary ImguiUi could be constructed but not registered (i.e. its WndProc was not yet made context aware),
        // and if the wrong context is currently active at WndProc invocation, there is a logical error.
        HWND hWnd = glfwGetWin32Window(window);

        // Sanity check: the window handle is the same for glfw and ImGui
        assert(hWnd == (HWND)ImGui::GetMainViewport()->PlatformHandleRaw);

        //std::cerr << "Configuring WndProc for window " << hWnd
        //    << " with user data " << mUserData.get()
        //    << " on thread " << std::this_thread::get_id()
        //    << "\n";

        mUserData->mPreviousWndProc = (WNDPROC)::GetWindowLongPtrW(hWnd, GWLP_WNDPROC);
        assert(mUserData->mPreviousWndProc != 0); // would like to assert that it is the function ImGui_ImplGlfw_WndProc
                                                  // But we do not see the symbol, only declared in a cpp file.
                    
        LONG_PTR userdata = ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);
        // We set user data to associate the ImGui context to the window procedure
        // the fact that the userdata was not used already simplifies things.
        assert(userdata == 0);
        ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)mUserData.get());

        ::SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)contextAwareWndProc);
#endif

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
    // Cannot scope the context here: it needs to remain active until renderBackend() is done.
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
