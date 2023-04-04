#include "Widgets.h"

#include <imgui.h>


namespace ad {
namespace imguiui {


void addCheckbox(const char * aLabel, std::atomic<bool> & aValue)
{
    bool value = aValue;
    ImGui::Checkbox(aLabel, &value);
    aValue = value;
}


} // namespace imguiui
} // namespace ad
