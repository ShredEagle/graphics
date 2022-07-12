#pragma once

#include <imgui.h>

#include <graphics/ApplicationGlfw.h>


namespace ad {
namespace imgui {

class ImguiUi
{
public:
    ImguiUi(const graphics::ApplicationGlfw & aApplication);

    ~ImguiUi();

    void newFrame() const;
    void render() const;

    bool isCapturingKeyboard() const;
    bool isCapturingMouse() const;

private:
    // Could be pimpled
    ImGuiIO & mIo;
};


} // namespace gltfviewer
} // namespace ad
