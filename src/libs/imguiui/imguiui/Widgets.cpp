#include "Widgets.h"

#include <imgui.h>


namespace ad {
namespace imguiui {


bool addCheckbox(const char * aLabel, bool & aValue)
{
    ImGui::Checkbox(aLabel, &aValue);
    return aValue;
}


bool addCheckbox(const char * aLabel, std::atomic<bool> & aValue)
{
    bool value = aValue;
    addCheckbox(aLabel, value);
    aValue = value;
    return value;
}


} // namespace imguiui
} // namespace ad
