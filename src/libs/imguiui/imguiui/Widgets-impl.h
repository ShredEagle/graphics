#pragma once


#include "Widgets.h"

#include <imgui.h>


namespace ad {
namespace imguiui {


template <class T_enumeration, std::size_t N_spanExtent>
void addCombo(const char * aLabel,
              T_enumeration & aValue,
              const std::span<const T_enumeration, N_spanExtent> & aAvailableValues)
{
    using graphics::to_string;

    static const ImGuiComboFlags flags = 0;
    // Pass in the preview value visible before opening the combo (it could be anything)
    const std::string combo_preview_value = to_string(aValue);
    if (ImGui::BeginCombo(aLabel, combo_preview_value.c_str(), flags))
    {
        Guard scopeCombo([]()
        {
            ImGui::EndCombo();
        });

        for (unsigned int n = 0; n < std::size(aAvailableValues); n++)
        {
            T_enumeration candidate = aAvailableValues[n];
            const bool isSelected = (aValue == candidate);
            if (ImGui::Selectable(to_string(candidate).c_str(), isSelected))
            {
                aValue = candidate;
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
    }
}


} // namespace imguiui
} // namespace ad